#include "pch.h"
#include "../ShogiStudyThird/kyokumen.h"
#include "../ShogiStudyThird/move_gen.h"
#include "../ShogiStudyThird/usi.h"
#include <algorithm>

using strv = std::vector<std::string>;

bool checkStrings(strv a, strv b) { //a=b
	for (auto& str : b) {
		auto result = std::find(a.begin(), a.end(), str);
		if (result == a.end()) return false;
		else a.erase(result);
	}
	if (a.empty()) return true;
	else return false;
}

bool checkStringsUnion(strv u, strv a, strv b) { // u=a∪b ∧ a∩b=∅
	for (auto& str : a) {
		auto result = std::find(u.begin(), u.end(), str);
		if (result == u.end()) return false;
		else u.erase(result);
	}
	for (auto& str : b) {
		auto result = std::find(u.begin(), u.end(), str);
		if (result == u.end()) return false;
		else u.erase(result);
	}
	if (u.empty()) return true;
	else return false;
}

bool genMoveCheck(std::string parent_sfen, std::string child_moves) {
	Kyokumen k(usi::split(parent_sfen,' '));
	const auto moves = MoveGenerator::genMove(new SearchNode(Move(0,0,false)), k);

}

TEST(KyokumenTest, proceed) {
	Kyokumen k;

  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}