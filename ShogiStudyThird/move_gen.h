#pragma once
#include "node.h"
#include "kyokumen.h"
#include "bb_kiki.h"

class MoveGenerator {
public:
	static void genAllMove(SearchNode* parent, const Kyokumen&);
	static void genMove(SearchNode* parent, const Kyokumen&);
	static const std::pair<std::vector<Move>,bool> genCapMove(const Move move, const Kyokumen&);
	static std::vector<Move> genNocapMove(SearchNode* parent, const Kyokumen&);
};