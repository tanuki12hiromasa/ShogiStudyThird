#pragma once
#include "tree.h"
#include "josekioption.h"
#include "sqlite/sqlite3.h"
#include "node.h"

class JosekiDataBase {
//定跡書き出し用
public:
	void replaceNodeWithPath(SearchNode* node, std::string path);
	int replaceNodeWithParent(SearchNode* node, int parentId);
	//conditionsの条件に指定したノードで最初に出てきたもののIDを返す
	int getIndex(std::string conditions);

	//データベースへ書き出し
	void josekiOutputToDataBaseWithPath(SearchNode* node, std::string path);
	void josekiOutputToDataBaseWithParent(SearchNode* node, int parentId);
	void josekiOutput(SearchNode* node, int parentID = -1);

private:


//定跡読み込み用
public:
	//historyの最後のノードの子ノードから最も評価が高いものを出力する
	bool getBestMoveFromDB(std::vector<SearchNode*>history);
	//treeにデータベースから定跡を読み込む
	void josekiInputFromDB(SearchTree* tree);

private:
	//parentidを貰い、parentnodeにノードを再帰的に格納していく。
	void yomikomiRecursiveFromDB(SearchNode* parentnode, const size_t parentid);

	class SelectStmt {
	public:
		SelectStmt(sqlite3* db, std::string columnsName, std::string tableName, std::string conditions);
		~SelectStmt();
		bool step();

		int getInt(int number);
		double getDouble(int number);
	private:
		sqlite3_stmt* stmt = nullptr;
		sqlite3* db;
	};
	struct nodedata {
		int id;
		Move move;
		SearchNode::State status;
		double eval;
		double depth;
		nodedata(int _id, Move _move, SearchNode::State _status, double _eval, double _depth) :id(_id), move(_move), status(_status), eval(_eval), depth(_depth) {

		}
	};

//定跡利用準備
public:
	JosekiDataBase();
	~JosekiDataBase();
	void open();
	void close();
	JosekiOption option;
	

private:
	sqlite3* db;
	std::string tableName;

	const char* tableContentText = 
		"("
			"id integer primary key"		","
			"parentid integer not null"		","
			"move int not null"				","
			"status int not null"			","
			"eval real not null"			","
			"depth real not null"			","
			
			"unique(parentid,move)"
		")"
	;
	
	void createTable();
};
