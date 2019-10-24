#pragma once
#include "move_gen.h"

class ShogiTest {
	using strv = std::vector<std::string>;
public:
	static bool checkStrings(strv a, strv b);
	static bool checkStringsUnion(strv u, strv a, strv b);
	static bool genMoveCheck(std::string parent_sfen, std::string child_moves);
	static bool genCapMoveCheck(std::string parent_sfen);
	static void test();
};