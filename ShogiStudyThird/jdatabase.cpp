#include "jdatabase.h"
#include <string>
#include <iostream>
#include <sstream>
#include "node.h"
#include "usi.h"

static const int BUFFERSIZE = 256;

static int callback(void* _, int argc, char** argv, char** columnName) {
	for (int i = 0; i < argc; i++)
		printf("%s = %s\n", columnName[i], argv[i] ? argv[i] : "null");
	return SQLITE_OK;
}

JosekiDataBase::JosekiDataBase(){
	option.addOption("joseki_database_folder", "string", "joseki");
	option.addOption("joseki_database_name", "string", "test.db");
	option.addOption("joseki_database_table_name", "string", "tablename");
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

void JosekiDataBase::josekiOutputToDataBaseWithParent(SearchNode* node, int parentId) {
	int nextParentId = replaceNodeWithParent(node, parentId);
	//std::cout << "id:" << nextParentId << std::endl;
	for (auto& cn : node->children) {
		josekiOutputToDataBaseWithParent(&cn, nextParentId);
	}
}

void JosekiDataBase::josekiOutput(SearchNode* node, int parentID){
	std::vector<std::thread> thr;
	int nextparent = replaceNodeWithParent(node, parentID);

	for (auto& cn:node->children) {
		thr.push_back(std::thread(&JosekiDataBase::josekiOutputToDataBaseWithParent, this,&cn, nextparent));
	}
	for (int i = 0; i < thr.size();++i) {
		thr[i].join();
	}
}


int JosekiDataBase::getIndex(std::string conditions) {
	SelectStmt ss(db, "id", tableName, conditions);
	ss.step();
	return ss.getInt(0);
}



JosekiDataBase::SelectStmt::SelectStmt(sqlite3* db, std::string columnsName, std::string tableName, std::string conditions) {
	char sql[BUFFERSIZE];
	sprintf_s(sql, BUFFERSIZE, "select %s from %s where %s", columnsName.c_str(), tableName.c_str(), conditions.c_str());
	//std::cout << sql << std::endl;
	auto status = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	this->db = db;
}
JosekiDataBase::SelectStmt::~SelectStmt() {
	sqlite3_finalize(stmt);
}

bool JosekiDataBase::SelectStmt::step()
{
	return	sqlite3_step(stmt) == SQLITE_ROW;
}

int JosekiDataBase::SelectStmt::getInt(int number) {
	return sqlite3_column_int(stmt, number);
}
double JosekiDataBase::SelectStmt::getDouble(int number) {
	return sqlite3_column_double(stmt, number);
}

//データベースから最善手を取り出す
bool JosekiDataBase::getBestMoveFromDB(std::vector<SearchNode*> his) {
	int parentID = -1;
	bool bestmoveExist = true;
	for (int i = 0; i < his.size() && bestmoveExist; ++i) {
		std::cout << his[i]->move.toUSI() << std::endl;

		//親ノードのIDから子ノードを取り出す
		SelectStmt ss(db, "id,move,status,eval,depth", tableName, "parentid = " + std::to_string(parentID));

		//子ノードがあるだけ続ける
		while (true) {
			if (ss.step() == false) {
				//一致する子ノードが無いことになるので終了
				bestmoveExist = false;
				break;
			}
			//もし一致したらparentIDを変更して次へ
			if (his[i]->move.getU() == ss.getInt(1)) {
				parentID = ss.getInt(0);
				break;
			}
		}
	}

	//最善手を探す
	if (bestmoveExist) {
		SelectStmt ss(db, "id,move,status,eval,depth", tableName, "parentid = " + std::to_string(parentID));
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
}

void JosekiDataBase::josekiInputFromDB(SearchTree* tree) {
	if (!option.getC("joseki_input_on")) {
		//return;
	}

	SearchNode* nextRoot = new SearchNode;

	SelectStmt ss(db,"id,move,status,eval,depth",tableName,"parentid = -1 and move = 12384");
	ss.step();

	//ルートノード自体の値を読み込み
	Move move(ss.getInt(1));
	SearchNode::State status = (SearchNode::State)ss.getInt(2);
	double eval = ss.getDouble(3);
	double depth = ss.getDouble(4);
	nextRoot->restoreNode(move, status, eval, depth);

	//yomikomiRecursiveFromDB(nextRoot,ss.getInt(0));

	//子ノードの数だけ再帰的に読みこむ
	SelectStmt sschildren(db,"id,move,status,eval,depth", tableName, "parentid = " + std::to_string(ss.getInt(0)));
	std::vector<nodedata> childrenData;
	while (sschildren.step()) {
		nodedata nd(sschildren.getInt(0), Move(sschildren.getInt(1)), (SearchNode::State)sschildren.getInt(2), sschildren.getDouble(3), sschildren.getDouble(4));
		childrenData.push_back(nd);
	}
	//childrenにセットするために配列にし、実際に値も挿入
	std::vector<std::thread> thr;
	//std::vector<JosekiDataBase>jdbs;
	SearchNode* list = new SearchNode[childrenData.size()];
	for (int i = 0; i < childrenData.size(); ++i) {
		/*JosekiDataBase jdb;
		jdb.open();
		jdbs.push_back(jdb);
		jdbs.back().option = option;
		jdbs.back().open();*/
		list[i].restoreNode(childrenData[i].move, childrenData[i].status, childrenData[i].eval, childrenData[i].depth);
		//thr.push_back(std::thread(&JosekiDataBase::yomikomiRecursiveFromDB, &jdbs.back(), &list[i], childrenData[i].id));
		thr.push_back(std::thread(&JosekiDataBase::yomikomiRecursiveFromDB, this, &list[i], childrenData[i].id));
	}
	std::cout << "thread num:" << thr.size() << std::endl;
	for (int i = 0; i < thr.size(); ++i) {
		thr[i].join();
		//jdbs[i].close();
	}
	nextRoot->children.setChildren(list, childrenData.size());

	//std::cout << "info next pv " << nextRoot->move.toUSI() << " cp " << nextRoot->eval << " depth " << nextRoot->mass << std::endl;
	tree->setRoot(nextRoot);
	std::cout << nextRoot->mass << std::endl;
	std::cout << nextRoot->eval << std::endl;
	std::cout << nextRoot->move.toUSI() << std::endl;
}


void JosekiDataBase::yomikomiRecursiveFromDB(SearchNode* parentnode, const size_t parentid) {
	SelectStmt ss(db, "id,move,status,eval,depth", tableName, "parentid = " + std::to_string(parentid));

	std::vector<nodedata> children;

	//必要な値を全て取り出し
	while (ss.step()) {
		nodedata node(ss.getInt(0),Move(ss.getInt(1)), (SearchNode::State)ss.getInt(2), ss.getDouble(3), ss.getDouble(4));
		children.push_back(node);
	}
	//ss.~SelectStmt();

	//childrenにセットするために配列にし、実際に値も挿入
	SearchNode* list = new SearchNode[children.size()];
	for (int i = 0; i < children.size();++i) {
		list[i].restoreNode(children[i].move, children[i].status, children[i].eval, children[i].depth);
		yomikomiRecursiveFromDB(&list[i], children[i].id);
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

	sqlite3_config(SQLITE_CONFIG_SERIALIZED);
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

int JosekiDataBase::replaceNodeWithParent(SearchNode* node, int parentId){
	char sql[BUFFERSIZE];
	sprintf_s(sql, BUFFERSIZE, "insert into %s (parentid,move,status,eval,depth) values(%d,%d,%d,%f,%f) on conflict(parentid,move) do update set status = %d , eval = %f , depth = %f", tableName.c_str(), parentId, (int)node->move.getU(), (int)node->getState(), (double)node->eval, (double)node->mass, (int)node->getState(), (double)node->eval, (double)node->mass);
	//std::cout << sql << std::endl;
	char* errorMessage;
	auto status = sqlite3_exec(db, sql, callback, nullptr, &errorMessage);
	if (status != SQLITE_OK) {
		std::cout << "replace:" << errorMessage << std::endl;
	}
	
	sprintf_s(sql, BUFFERSIZE, "select id from %s where parentid = ? and move = ?", tableName.c_str());
	//std::cout << sql << std::endl;
	sqlite3_stmt *stmt = nullptr;
	status = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, parentId);
	sqlite3_bind_int(stmt, 2, (int)node->move.getU());
	sqlite3_step(stmt);
	int id = sqlite3_column_int(stmt, 0);
	if (status != SQLITE_OK) {
		std::cout << "select:" << errorMessage << std::endl;
	}
	
	sqlite3_finalize(stmt);
	return (int)id;
}
