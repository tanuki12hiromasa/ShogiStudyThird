#pragma once
#include "tree.h"
#include "josekioption.h"
#include "sqlite/sqlite3.h"
#include "node.h"

class JosekiDataBase {
public:
private:
	class Stmt{
	public:
		Stmt(sqlite3 * db, std::string sql);
		~Stmt();
		bool step();

		int getInt(int number);
		size_t getSize_t(int number);
		double getDouble(int number);
		void bind(int num, int eval);
		void bind(int num, size_t eval);
		void bind(int num, double eval);

		//void bindDouble(int num, double eval);
		void reset();
	private:
		sqlite3_stmt* stmt = nullptr;
	};
	struct nodedata {
		size_t id;
		Move move;
		SearchNode::State status;
		double eval;
		double depth;
		nodedata(size_t _id, Move _move, SearchNode::State _status, double _eval, double _depth) :id(_id), move(_move), status(_status), eval(_eval), depth(_depth) {

		}
	};

//定跡書き出し用
public:
	void josekiOutput(SearchNode* node, size_t parentID = 0);
private:
	void replaceNodeWithPath(SearchNode* node, std::string path);
	size_t replaceNodeWithParent(SearchNode* node, Stmt* insert, Stmt* select, size_t parentId);
	//conditionsの条件に指定したノードで最初に出てきたもののIDを返す
	size_t getIndex(std::string conditions);

	//データベースへ書き出し
	void josekiOutputToDataBaseWithPath(SearchNode* node, std::string path);
	void josekiOutputToDataBaseWithParent(SearchNode* node, Stmt* insert, Stmt* select, size_t parentId);


//定跡読み込み用
public:
	//historyの最後のノードの子ノードから最も評価が高いものを出力する
	bool getBestMoveFromDB(std::vector<SearchNode*>history);
	//treeにデータベースから定跡を読み込む
	void josekiInputFromDB(SearchTree* tree);

private:
	//parentidを貰い、parentnodeにノードを再帰的に格納していく。
	void yomikomiRecursiveFromDB(SearchNode* parentnode, Stmt* ss, size_t parentid);



//定跡利用準備
public:
	JosekiDataBase();
	~JosekiDataBase();
	void init();
	JosekiOption option;
	

private:
	void open();
	void close();
	sqlite3* db;
	std::string tableName;
	bool isOpen = false;
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
