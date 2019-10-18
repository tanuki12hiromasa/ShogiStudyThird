#pragma once
#include "node.h"
#include "kyokumen.h"
#include "bb_kiki.h"

class MoveGenerator {
public:
	static std::vector<SearchNode*> genMove(SearchNode* parent, const Kyokumen&);
	static std::vector<SearchNode*> genCapMove(SearchNode* parent, const Kyokumen&);
	static std::vector<SearchNode*> genNocapMove(SearchNode* parent, const Kyokumen&);
};