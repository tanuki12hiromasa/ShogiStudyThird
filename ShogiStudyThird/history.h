#pragma once
#include "node.h"

class History {
public:
	void setRoot(SearchNode*);
	void add(SearchNode*);
	SearchNode* back();
	SearchNode* endNode()const;
	bool isEndRoot()const;

	std::vector<SearchNode*> his;
	SearchNode* root;

};