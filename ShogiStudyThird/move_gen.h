#pragma once
#include "node.h"
#include "kyokumen.h"

class MoveGenerator {
public:
	std::vector<SearchNode*> genMove(SearchNode* parent, const Kyokumen&);
	std::vector<SearchNode*> genCapMove(SearchNode* parent, const Kyokumen&);
	std::vector<SearchNode*> genNocapMove(SearchNode* parent, const Kyokumen&);
};