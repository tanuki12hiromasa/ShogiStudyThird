#pragma once
#include "node.h"
#include "kyokumen.h"
#include "bb_kiki.h"

class MoveGenerator {
public:
	static std::vector<Move> genAllMove(Move& move, const Kyokumen&);
	static std::vector<Move> genMove(Move& move, const Kyokumen&);
	static std::pair<std::vector<Move>, std::vector<Move>> genMoveWithRemoved(Move& move, const Kyokumen&);
	static std::vector<Move> genCapMove(Move& move, const Kyokumen&);
	static std::vector<Move> genNocapMove(Move& move, const Kyokumen&);
};