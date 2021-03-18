#pragma once
#include "tree.h"
#include "josekioption.h"
#include "sqlite/sqlite3.h"

class JosekiDataBase {
public:
	JosekiDataBase();
	void open();
	void close();
	void replaceNodeWithPath(SearchNode* node, std::string path);
	int replaceNodeWithParent(SearchNode* node, int parentId);
	JosekiOption option;
	std::string selectNodeFromPath(std::string history);
	std::string selectNodeFromPath(std::vector<std::string> history);
	std::vector<int>getChildIndex(int index);
private:
	sqlite3* db;
	std::string tableName;
	struct column {
		column(std::string _name, std::string _type,bool _primary, bool _unique, bool _notnull,std::string _option = "") {
			name = _name;
			type = _type;
			primary = _primary;
			unique = _unique;
			notnull = _notnull;
			option = _option;
		}
		std::string name;
		std::string type;
		bool primary;
		bool unique;
		bool notnull;
		std::string option;
	};
	void createTable(std::vector<column> columns);
};
