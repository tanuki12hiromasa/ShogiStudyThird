#pragma once
#include "move_gen.h"
#include "commander.h"

class ShogiTest {
	using strv = std::vector<std::string>;
public:
	static bool checkKyokumenProceed(std::string sfen, Move m, std::string correct);
	static bool checkStrings(strv a, strv b);
	static bool checkStringsUnion(strv u, strv a, strv b);
	static bool checkStringsInclusion(strv u, strv a);
	static void coutStringsDiff(const strv& cor, const strv& s);
	static bool genMoveCheck(std::string parent_sfen, std::string child_moves);
	static bool genMoveCheck(std::string parent_sfen, Move pmove, std::string child_moves);
	static bool genCapMoveCheck(std::string parent_sfen);
	static bool checkFeature(std::string usiposition);
	static bool checkRecede(std::string sfen,const int depth);
	static bool checkRecedeR(Kyokumen& k, Feature& f, SearchNode* p, const int depth);
	static void test();
};