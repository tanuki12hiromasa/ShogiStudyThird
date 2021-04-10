#include "jdatabase.h"
#include <string>
#include <iostream>
#include <sstream>
#include "node.h"
#include "sqlite/sqlite3.h"
#include "usi.h"

static const int BUFFERSIZE = 256;

static int callback(void* _, int argc, char** argv, char** columnName) {
	for (int i = 0; i < argc; i++)
		printf("%s = %s\n", columnName[i], argv[i] ? argv[i] : "null");
	return SQLITE_OK;
}

JosekiDataBase::JosekiDataBase(){
	option.addOption("joseki_database_on", "check", "false");
	option.addOption("joseki_database_folder", "string", "joseki");
	option.addOption("joseki_database_name", "string", "test.db");
	option.addOption("joseki_database_table_name", "string", "tablename");
	option.addOption("joseki_database_multi_thread_on", "check", "false");
}

JosekiDataBase::~JosekiDataBase(){
	close();
}

void JosekiDataBase::josekiOutputToDataBaseWithPath(SearchNode* node, std::string path) {
	if (node->move.toUSI() != "nullmove") {
		path += node->move.toUSI() + " ";
	}
	replaceNodeWithPath(node, path);
	for (auto& cn : node->children) {
		josekiOutputToDataBaseWithPath(&cn, path);
	}
}

void JosekiDataBase::josekiOutputToDataBaseWithParent(SearchNode* node, Stmt* insert, Stmt* select, size_t parentId) {
	size_t nextParentId = replaceNodeWithParent(node,insert,select, parentId);
	//std::cout << "id:" << nextParentId << std::endl;
	for (auto& cn : node->children) {
		josekiOutputToDataBaseWithParent(&cn,insert,select, nextParentId);
	}
}

void JosekiDataBase::josekiOutput(SearchNode* node, size_t parentID){
	if (!isOpen) {
		open();
	}

	Stmt *insert = new Stmt(db, "insert into " + tableName + "(parentid,move,status,eval,depth) values(?,?,?,?,?) on conflict(parentid,move) do update set status = ? , eval = ? , depth = ?");
	Stmt *select = new Stmt(db, "select id from " + tableName + " where parentid = ? and move = ?");
	
	//sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

	
	if (option.getC("joseki_database_multi_thread_on")) {
		std::vector<std::thread> thr;
		int nextparent = replaceNodeWithParent(node, insert, select, parentID);
		for (auto& cn:node->children) {
			thr.push_back(std::thread(&JosekiDataBase::josekiOutputToDataBaseWithParent, this, &cn, insert, select, nextparent));
		}
		for (int i = 0; i < thr.size();++i) {
			thr[i].join();
		}
	}
	else {
		josekiOutputToDataBaseWithParent(node, insert, select, parentID);
	}

	close();
}


size_t JosekiDataBase::getIndex(std::string conditions) {
	Stmt ss(db, "select id from " + tableName + " where " + conditions);
	ss.step();
	return ss.getSize_t(0);
}



JosekiDataBase::Stmt::Stmt(sqlite3* db, std::string sqltext) {
	static int i = 0;
	auto status = sqlite3_prepare_v2(db, sqltext.c_str(), -1, &stmt, NULL);
	if (status != SQLITE_OK)
	{
		printf("sql[%s] error! [%s]\n", sqltext.c_str(),sqlite3_errmsg(db));   // [unable to open database file]
	}
	++i;
}
JosekiDataBase::Stmt::~Stmt() {
	sqlite3_finalize(stmt);
}

bool JosekiDataBase::Stmt::step()
{
	return	sqlite3_step(stmt) == SQLITE_ROW;
}

int JosekiDataBase::Stmt::getInt(int number) {
	return sqlite3_column_int(stmt, number);
}

size_t JosekiDataBase::Stmt::getSize_t(int number){
	return sqlite3_column_int64(stmt,number);
}
double JosekiDataBase::Stmt::getDouble(int number) {
	return sqlite3_column_double(stmt, number);
}

void JosekiDataBase::Stmt::bind(int num, int eval){
	sqlite3_bind_int(stmt, num, eval);
}

void JosekiDataBase::Stmt::bind(int num, size_t eval){
	sqlite3_bind_int64(stmt, num, eval);
}

void JosekiDataBase::Stmt::bind(int num, double eval){
	sqlite3_bind_double(stmt, num, eval);
}

void JosekiDataBase::Stmt::reset(){
	sqlite3_reset(stmt);
}

//データベースから最善手を取り出す
bool JosekiDataBase::getBestMoveFromDB(std::vector<SearchNode*> his) {
	if (!isOpen) {
		open();
	}

	size_t parentID = 0;
	bool bestmoveExist = true;
	Stmt ss(db, "select id,move,status,eval,depth from " + tableName + " parentid = ?");
	for (int i = 0; i < his.size() && bestmoveExist; ++i) {
		ss.bind(1,parentID);
		std::cout << his[i]->move.toUSI() << std::endl;

		//親ノードのIDから子ノードを取り出す

		//子ノードがあるだけ続ける
		while (true) {
			if (ss.step() == false) {
				//一致する子ノードが無いことになるので終了
				bestmoveExist = false;
				break;
			}
			//もし一致したらparentIDを変更して次へ
			if (his[i]->move.getU() == ss.getInt(1)) {
				parentID = ss.getSize_t(0);
				break;
			}
		}
		ss.reset();
	}

	//最善手を探す
	if (bestmoveExist) {
		ss.bind(1,parentID);
		uint16_t bestU = 12384;
		double bestEval = -9999999;
		while (ss.step()) {
			auto eval = ss.getDouble(3);
			if (bestEval < eval) {
				bestEval = eval;
				bestU = ss.getInt(1);
			}
		}

		if (bestU != 12384) {
			std::cout << "bestmove " << Move(bestU).toUSI() << std::endl;
		}
		else {
			bestmoveExist = false;
		}
	}

	close();
}

void JosekiDataBase::josekiInputFromDB(SearchTree* tree) {
	if (option.getC("joseki_database_on") == false) {
		return;
	}
	if (!isOpen) {
		open();
	}

	SearchNode* nextRoot = new SearchNode;

	Stmt ss(db,"select id,move,status,eval,depth from " + tableName + " where parentid = 0 and move = 12384");
	ss.step();
	//ルートノード自体の値を読み込み
	size_t parentID = ss.getSize_t(0);
	Move move(ss.getInt(1));
	SearchNode::State status = (SearchNode::State)ss.getInt(2);
	double eval = ss.getDouble(3);
	double depth = ss.getDouble(4);
	nextRoot->restoreNode(move, status, eval, depth);


	if (option.getC("joseki_database_multi_thread_on")) {
		//子ノードの数だけ再帰的に読みこむ
		Stmt sschildren(db, "select id,move,status,eval,depth from " + tableName + " where parentid = " + std::to_string(parentID));
		std::vector<nodedata> childrenData;
		while (sschildren.step()) {
			nodedata nd(sschildren.getSize_t(0), Move(sschildren.getInt(1)), (SearchNode::State)sschildren.getInt(2), sschildren.getDouble(3), sschildren.getDouble(4));
			childrenData.push_back(nd);
		}

		//childrenにセットするために配列にし、実際に値も挿入
		std::vector<std::thread> thr;
		std::vector<Stmt*>sss;
		SearchNode* list = new SearchNode[childrenData.size()];
		for (int i = 0; i < childrenData.size(); ++i) {
			Stmt* ss = new Stmt(db, "select id,move,status,eval,depth from " + tableName + " where parentid = ?");
			list[i].restoreNode(childrenData[i].move, childrenData[i].status, childrenData[i].eval, childrenData[i].depth);
			thr.push_back(std::thread(&JosekiDataBase::yomikomiRecursiveFromDB, this, &list[i], ss, childrenData[i].id));
			sss.push_back(ss);
		}
		std::cout << "thread num:" << thr.size() << std::endl;
		for (int i = 0; i < thr.size(); ++i) {
			thr[i].join();
		}
		nextRoot->children.setChildren(list, childrenData.size());
	}
	else {
		Stmt* stmt = new Stmt(db, "select id,move,status,eval,depth from " + tableName + " where parentid = ?");
		yomikomiRecursiveFromDB(nextRoot, stmt, parentID);
	}	

	//std::cout << "info next pv " << nextRoot->move.toUSI() << " cp " << nextRoot->eval << " depth " << nextRoot->mass << std::endl;
	tree->setRoot(nextRoot);
	std::cout << nextRoot->mass << std::endl;
	std::cout << nextRoot->eval << std::endl;
	std::cout << nextRoot->move.toUSI() << std::endl;
}


void JosekiDataBase::yomikomiRecursiveFromDB(SearchNode* parentnode,Stmt* ss, size_t parentid) {
	//Stmt ss(db, "id,move,status,eval,depth", tableName, "parentid = " + std::to_string(parentid));

	std::vector<nodedata> children;

	ss->bind(1,parentid);

	//必要な値を全て取り出し
	while (ss->step()) {
		nodedata node(ss->getSize_t(0),Move(ss->getInt(1)), (SearchNode::State)ss->getInt(2), ss->getDouble(3), ss->getDouble(4));
		children.push_back(node);
	}
	//ss.~Stmt();
	ss->reset();

	//childrenにセットするために配列にし、実際に値も挿入
	SearchNode* list = new SearchNode[children.size()];
	for (int i = 0; i < children.size();++i) {
		list[i].restoreNode(children[i].move, children[i].status, children[i].eval, children[i].depth);
		yomikomiRecursiveFromDB(&list[i],ss, children[i].id);
	}
	parentnode->children.setChildren(list, children.size());

}


void JosekiDataBase::open(){
	std::string dbFolder = option.getS("joseki_database_folder");
	std::string dbName = option.getS("joseki_database_name");
	char* errorMessage;
	std::string filePath = dbFolder + "\\" + dbName;
	auto status = sqlite3_open(filePath.c_str(), &db);
	if (status != SQLITE_OK)
	{
		std::cout << "error for opening db." << std::endl;
	}
	else
	{
		std::cout << "open successed!" << std::endl;
	}


	//テーブルの存在チェック
	tableName = option.getS("joseki_database_table_name");
	auto exist = sqlite3_exec(db, ("select count(*) from sqlite_master where type=\"table\" and name=\"" + tableName + "\"").c_str(), callback, nullptr, &errorMessage);

	//テーブルが無かったらテーブルを作る
	if (!exist) {
		// create table
		createTable();
	}
	else {
		std::cout << "table:" << option.getS("joseki_database_table_name") << "is already exist." << std::endl;
	}

	//トランザクション開始
	auto ret = sqlite3_exec(db, "BEGIN;", nullptr, nullptr, &errorMessage);
	if (ret != SQLITE_OK) {
		std::cout << "begin:" << errorMessage << std::endl;
	}

	isOpen = true;
}

void JosekiDataBase::close(){
	//トランザクション終了
	char* errorMessage;
	auto ret = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errorMessage);
	if (ret != SQLITE_OK) {
		std::cout << "commit:" << errorMessage << std::endl;
		sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errorMessage);
	}

	sqlite3_close(db);
	isOpen = false;
}

void JosekiDataBase::createTable() {
	std::string createTable = "create table " + tableName + " ";

	createTable += tableContentText;

	createTable += "";
	std::cout << createTable << std::endl;
	char* errorMessage;
	auto status = sqlite3_exec(db, createTable.c_str(), nullptr, nullptr, &errorMessage);
	if (status != SQLITE_OK) {
		std::cout << "create:" << errorMessage << std::endl;
	}
}

void JosekiDataBase::replaceNodeWithPath(SearchNode* node,std::string path)
{
	char sql[BUFFERSIZE];
	sprintf_s(sql,BUFFERSIZE, "replace into %s (status,move,eval,depth,path) values(%d,%d,%f,%f,\"%s\")", tableName.c_str(), (int)node->getState(), (int)node->move.getU(), (double)node->eval, (double)node->mass, path.c_str());
	//std::cout << sql << std::endl;
	char* errorMessage;
	auto status = sqlite3_exec(db, sql, callback, nullptr, &errorMessage);
	if (status != SQLITE_OK) {
		std::cout << "replace:" << errorMessage << std::endl;
	}
}

size_t JosekiDataBase::replaceNodeWithParent(SearchNode* node, Stmt* insert, Stmt* select, size_t parentId){
	char sql[BUFFERSIZE];
	//sprintf_s(sql, BUFFERSIZE, "insert into %s (parentid,move,status,eval,depth) values(%d,%d,%d,%f,%f) on conflict(parentid,move) do update set status = %d , eval = %f , depth = %f", tableName.c_str(), parentId, (int)node->move.getU(), (int)node->getState(), (double)node->eval, (double)node->mass, (int)node->getState(), (double)node->eval, (double)node->mass);
	int rc;
	//"insert into %s(parentid,move,status,eval,depth) values(?,?,?,?,?) on conflict(parentid,move) do update set status = ? , eval = ? , depth = ?"
	int n = 1;
	insert->bind(n, parentId); ++n;
	insert->bind(n, node->move.getU()); ++n;
	insert->bind(n, (int)node->getState()); ++n;
	insert->bind(n, node->eval); ++n;
	insert->bind(n, node->mass); ++n;
	insert->bind(n, (int)node->getState()); ++n;
	insert->bind(n, node->eval); ++n;
	insert->bind(n, node->mass); ++n;
	
	insert->step();

	select->bind(1, parentId);
	select->bind(2, (int)node->move.getU());
	select->step();
	size_t id = select->getSize_t(0);

	insert->reset();
	select->reset();

	return id;
}
