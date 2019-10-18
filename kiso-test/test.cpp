#include "pch.h"
#include "gtest/gtest.h"
#include "../ShogiStudyThird/kyokumen.h"
#include "../ShogiStudyThird/move_gen.h"
#include "../ShogiStudyThird/usi.h"
#include <algorithm>

using strv = std::vector<std::string>;
__declspec(dllexport) int PullInMyLibrary() { return 0; }

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

void genMoveCheck(std::string parent_sfen, std::string child_moves) {
	strv ans = usi::split(child_moves, ' ');
	Kyokumen k(usi::split(parent_sfen,' '));
	const auto moves = MoveGenerator::genMove(new SearchNode(Move(0,0,false)), k);
	strv msv; for (const auto& m : moves)msv.push_back(m->move.toUSI());
	ASSERT_TRUE(checkStrings(ans, msv));
}

void genCapMoveCheck(std::string parent_sfen) {
	Kyokumen k(usi::split(parent_sfen, ' '));
	const auto moves = MoveGenerator::genMove(new SearchNode(Move()), k);
	strv msv; for (const auto& m : moves)msv.push_back(m->move.toUSI());
	const auto cmoves = MoveGenerator::genCapMove(new SearchNode(Move()), k);
	strv cmsv; for (const auto& m : cmoves)cmsv.push_back(m->move.toUSI());
	const auto nmoves = MoveGenerator::genNocapMove(new SearchNode(Move()), k);
	strv nmsv; for (const auto& m : nmoves)nmsv.push_back(m->move.toUSI());
	ASSERT_TRUE(checkStringsUnion(msv, cmsv, nmsv));
}

TEST(KyokumenTest, proceed) {
	BBkiki::init();
	ASSERT_NO_FATAL_FAILURE(genMoveCheck("position sfen 4k4/5+p3/5P3/9/9/9/9/9/4K4 b 2r2b4g4s4n4l16p 1", "4c4b 4c4b + 5i4h 5i4i 5i5h 5i6h 5i6i"));
	ASSERT_NO_FATAL_FAILURE(genCapMoveCheck("position sfen 4k4/5+p3/5P3/9/9/9/9/9/4K4 b 2r2b4g4s4n4l16p 1"));
	EXPECT_EQ(1, 1);
}