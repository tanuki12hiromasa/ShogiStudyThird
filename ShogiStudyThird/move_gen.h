#pragma once
#include "node.h"
#include "kyokumen.h"

class MoveGenerator {
public:
	static std::vector<SearchNode*> genMove(SearchNode* parent, const Kyokumen&);
	static std::vector<SearchNode*> genCapMove(SearchNode* parent, const Kyokumen&);
	static std::vector<SearchNode*> genNocapMove(SearchNode* parent, const Kyokumen&);
};