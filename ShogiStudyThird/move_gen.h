#pragma once
#include "node.h"
#include "kyokumen.h"
#include "bb_kiki.h"

class MoveGenerator {
public:
	static void genAllMove(SearchNode* parent, const Kyokumen&);
	static std::vector<Move> genMove(SearchNode* parent, const Kyokumen&);
	static std::vector<Move> genCapMove(Move& move, const Kyokumen&);
	static std::vector<Move> genNocapMove(Move& move, const Kyokumen&);
};