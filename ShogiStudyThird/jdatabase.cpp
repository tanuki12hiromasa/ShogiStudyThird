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
		std::vector<column> columns;
		if (true) {
			//columns.push_back(column("id",       "INTEGER", false, true,  true,"autoincrement"));
			columns.push_back(column("parentid", "INTEGER", true,  false, true));
			columns.push_back(column("move",     "INTEGER", true,  false, true));
			columns.push_back(column("status",   "INTEGER", false, false, true));
			columns.push_back(column("eval",     "REAL",    false, false, true));
			columns.push_back(column("depth",    "REAL",    false, false, true));
		}
		else {

		}
		createTable(columns);
	}
	else {
		std::cout << "table:" << option.getS("joseki_database_table_name") << "is already exist." << std::endl;
	}

	//トランザクション開始
	auto ret = sqlite3_exec(db, "BEGIN;", nullptr, nullptr, &errorMessage);
	if (ret != SQLITE_OK) {
		std::cout << "begin:" << errorMessage << std::endl;
	}
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

void JosekiDataBase::createTable(std::vector<column> columns) {
	std::vector<int>prime;
	std::string createTable = "create table " + tableName + " (";

	for (int i = 0; i < columns.size(); ++i) {
		if (i != 0) {
			createTable += ",";
		}

		createTable += columns[i].name;
		createTable += " " + columns[i].type;
		if (columns[i].notnull) {
			createTable += " not null";
		}
		if (columns[i].unique) {
			createTable += " unique";
		}
		createTable += " " + columns[i].option;
		if (columns[i].primary) {
			prime.push_back(i);
		}
	}

	if (prime.size() > 0) {
		createTable += ",primary key(";
		for (int i = 0; i < prime.size(); ++i) {
			if (i != 0) {
				createTable += ",";
			}
			createTable += columns[prime[i]].name;
		}
		createTable += ")";
	}

	createTable += ")";
	std::cout << createTable << std::endl;
	char* errorMessage;
	auto status = sqlite3_exec(db, createTable.c_str(), nullptr, nullptr, &errorMessage);
	if (status != SQLITE_OK) {
		std::cout << "create:" << errorMessage << std::endl;
	}
}

std::vector<int> JosekiDataBase::getChildIndex(int index){
	char sql[BUFFERSIZE];
	sprintf_s(sql, BUFFERSIZE, "select rowid from %s where parentid = ?", tableName.c_str());
	//std::cout << sql << std::endl;
	sqlite3_stmt* stmt = nullptr;
	auto status = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, index);
	std::vector<int> r;
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int id = sqlite3_column_int(stmt, 0);
		if (status != SQLITE_OK) {
			std::cout << "select:" << std::endl;
		}
		r.push_back(id);
	}
	sqlite3_finalize(stmt);
	return r;
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
	
	sprintf_s(sql, BUFFERSIZE, "select rowid from %s where parentid = ? and move = ?", tableName.c_str());
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

std::string JosekiDataBase::selectNodeFromPath(std::string history)
{
	return selectNodeFromPath(usi::split(history, ' '));
}


std::string JosekiDataBase::selectNodeFromPath(std::vector<std::string> history)
{
	std::stringstream ss;
	ss << "select * from " << tableName;
	return std::string();
}


