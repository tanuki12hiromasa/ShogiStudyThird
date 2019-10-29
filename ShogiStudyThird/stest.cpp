#include "stest.h"
#include "commander.h"
#include "usi.h"
#include <iostream>

bool ShogiTest::checkKyokumenProceed(std::string sfen, Move m, std::string correct) {
	Kyokumen k(usi::split(sfen, ' '));
	k.proceed(m);
	Kyokumen c(usi::split(correct, ' '));
	if (k != c) {
		std::cout << "error:proceed" << std::endl;
		std::cout << "correct : " << c.toSfen() << std::endl;
		std::cout << "kyokumen: " << k.toSfen() << std::endl;
		assert(0);
		return false;
	}
	std::cout << "test: proceed ok" << std::endl;
	return true;
}

bool ShogiTest::checkStrings(strv a, strv b) { //a=b
	for (auto& str : b) {
		auto result = std::find(a.begin(), a.end(), str);
		if (result == a.end()) return false;
		else a.erase(result);
	}
	if (a.empty()) return true;
	else return false;
}


bool ShogiTest::checkStringsUnion(strv u, strv a, strv b) { // u=a∪b ∧ a∩b=∅
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

bool ShogiTest::genMoveCheck(std::string parent_sfen, std::string child_moves) {
	strv ans = usi::split(child_moves, ' ');
	Kyokumen k(usi::split(parent_sfen, ' '));
	const auto moves = MoveGenerator::genMove(new SearchNode(Move()), k);
	strv msv; for (const auto& m : moves)msv.push_back(m->move.toUSI());
	if (checkStrings(ans, msv)) return true;
	else {
		for (const auto& s : msv)std::cout << s << ' ';
		std::cout << std::endl;
		assert(0);
		return false;
	} 
}

bool ShogiTest::genMoveCheck(std::string parent_sfen, Move pmove, std::string child_moves) {
	strv ans = usi::split(child_moves, ' ');
	Kyokumen k(usi::split(parent_sfen, ' '));
	const auto moves = MoveGenerator::genMove(new SearchNode(pmove), k);
	strv msv; for (const auto& m : moves)msv.push_back(m->move.toUSI());
	if (checkStrings(ans, msv)) return true;
	else {
		for (const auto& s : msv)std::cout << s << ' ';
		std::cout << std::endl;
		assert(0);
		return false;
	}
}

bool ShogiTest::genCapMoveCheck(std::string parent_sfen) {
	Kyokumen k(usi::split(parent_sfen, ' '));
	const auto moves = MoveGenerator::genMove(new SearchNode(Move()), k);
	strv msv; for (const auto& m : moves)msv.push_back(m->move.toUSI());
	const auto cmoves = MoveGenerator::genCapMove(new SearchNode(Move()), k);
	strv cmsv; for (const auto& m : cmoves)cmsv.push_back(m->move.toUSI());
	const auto nmoves = MoveGenerator::genNocapMove(new SearchNode(Move()), k);
	strv nmsv; for (const auto& m : nmoves)nmsv.push_back(m->move.toUSI());

	if (checkStringsUnion(msv, cmsv, nmsv)) { 
		std::cout << "gencapmove test ok" << std::endl;
		return true; 
	}
	else {
		std::cout << "genfull: ";
		for (const auto& s : msv)std::cout << s << ' ';
		std::cout << std::endl;
		std::cout << "gencap: ";
		for (const auto& s : cmsv)std::cout << s << ' ';
		std::cout << std::endl;
		std::cout << "gennocap: ";
		for (const auto& s : nmsv)std::cout << s << ' ';
		std::cout << std::endl;
		assert(0);
		return false;
	}
}

void ShogiTest::test() {
	using namespace std;
	BBkiki::init();
#if 1
	{
		std::string sfen = "position sfen 4k4/5+p3/5P3/9/9/9/9/9/4K4 b 2r2b4g4s4n4l16p 1";
		Kyokumen k(usi::split(sfen, ' '));
		Move m(29, 28, true);
		k.proceed(m);
		auto r = k.getGoteOuCheck();
		assert(!r.empty());
		string sfen2 = "position sfen lnsgkg1nl/1r3s1b1/ppppppppp/9/9/2P6/PP1PPPPPP/1B5R1/LNSGKGSNL b - 1";
		Move m2(70, 20, false);
		string ans = "position sfen lnsgkg1nl/1r3s1b1/ppppppBpp/9/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w P 1";
		checkKyokumenProceed(sfen2, m2, ans);
		string sfen3 = "position sfen lnsgkgsnl/1r5b1/ppppppppp/9/9/2P6/PP1PPPPPP/1B5R1/LNSGKGSNL w - 1";
		Move m3(18, 28, false);
		checkKyokumenProceed(sfen3, m3, sfen2);
	}
#endif;
	{
		string str5 = "position sfen lnsgkg1nl/1r3s1b1/ppppppBpp/9/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w P 1";
		string moves5 = "1c1d 2c2d 4c4d 5c5d 6c6d 7c7d 8c8d 9c9d 2a3c 4b3c 7a6b 7a7b 4a3a 4a3b 4a5b 6a5b 6a6b 6a7b 5a5b 5a6b 1a1b 9a9b 2b3a 2b3c 8b5b 8b6b 8b7b 8b9b";
		ShogiTest::genMoveCheck(str5, Move(70,20,false), moves5);
		ShogiTest::genCapMoveCheck(str5);
		string str4 = "position sfen lnsgkgsnl/1r7/pppppp1pp/6p2/9/7P1/PPbPPPP1P/1B1K3R1/LNSG1GSNL b p 1";
		string moves4 = "8i7g 8h7g 6h5h 6h7g 6h7h";
		ShogiTest::genMoveCheck(str4, moves4);
		string str3 = "position sfen lnsgkgsnl/1r7/pppppp1pp/6p2/9/7P1/PP+bPPPP1P/1B1K3R1/LNSG1GSNL b p 1";
		string moves3 = "8i7g 8h7g 6h5h 6h7g";
		ShogiTest::genMoveCheck(str3, moves3);
		ShogiTest::genCapMoveCheck(str3);
		ShogiTest::genMoveCheck("position sfen 4k4/5+p3/5P3/9/9/9/9/9/4K4 b 2r2b4g4s4n4l16p 1", "4c4b 4c4b+ 5i4h 5i4i 5i5h 5i6h 5i6i");
		ShogiTest::genCapMoveCheck("position sfen 4k4/5+p3/5P3/9/9/9/9/9/4K4 b 2r2b4g4s4n4l16p 1");
		string str1 = "position sfen l3k2nl/2g3gb1/3psppp1/prp5p/9/2P1R3P/PP1PPPP2/2G3S2/L1S1KG1NL b S2Pb2np 1";
		ShogiTest::genMoveCheck(str1, "P*2d P*2e P*2f P*2g P*2h S*1b S*1c S*1e S*1g S*1h S*2d S*2e S*2f S*2g S*2h S*3a S*3d S*3e S*3f S*3i S*4a S*4b S*4d S*4e S*4f S*4h S*5b S*5d S*5e S*5h S*6a S*6b S*6d S*6e S*6f S*6h S*6i S*7a S*7c S*7e S*7g S*8a S*8b S*8c S*8e S*8f S*8h S*8i S*9b S*9c S*9e S*9f S*9h 1f1e 3g3f 4g4f 6g6f 7f7e 8g8f 9g9f 2i1g 3h2g 7i6h 7i8h 4i3i 4i4h 4i5h 7h6h 7h7g 7h8h 5i4h 5i5h 5i6h 5i6i 1i1g 1i1h 9i9h 5f2f 5f3f 5f4f 5f5c 5f5c+ 5f5d 5f5e 5f6f");
		ShogiTest::genCapMoveCheck(str1);
		string str2 = "position sfen l3k2nl/2g2sgb1/3p+Nppp1/prp5p/9/2P1R3P/PP1PPPP2/2G3S2/L1S1KG1NL w S2Pbnp 1";
		string moves2 = "P*5b P*5d P*5e P*5h P*8a P*8b P*8c P*8e P*8f P*8h N*1b N*1c N*1e N*1g N*2d N*2e N*2f N*2g N*3a N*3d N*3e N*3f N*4a N*4d N*4e N*4f N*5b N*5d N*5e N*6a N*6b N*6d N*6e N*6f N*7a N*7c N*7e N*7g N*8a N*8b N*8c N*8e N*8f N*9b N*9c N*9e N*9f B*1b B*1c B*1e B*1g B*1h B*2d B*2e B*2f B*2g B*2h B*3a B*3d B*3e B*3f B*3i B*4a B*4d B*4e B*4f B*4h B*5b B*5d B*5e B*5h B*6a B*6b B*6d B*6e B*6f B*6h B*6i B*7a B*7c B*7e B*7g B*8a B*8b B*8c B*8e B*8f B*8h B*8i B*9b B*9c B*9e B*9f B*9h 1d1e 2c2d 3c3d 4c4d 6c6d 7d7e 9d9e 2a1c 4b3a 4b5c 3b3a 7b6b 7b7a 7b7c 7b8b 7b8c 5a4a 5a6a 1a1b 1a1c 9a9b 9a9c 2b1c 2b3a 8d8a 8d8b 8d8c 8d8e 8d8f 8d8g 8d8g+";
		ShogiTest::genMoveCheck(str2, moves2);
		ShogiTest::genCapMoveCheck(str2);
	}
#if 1
	{
		Commander com;
		std::cout << "initializing now..." << std::endl;
		com.paramInit();
		Evaluator::init();
		SearchTree& tree = com.tree;
		tree.rootPlayer.feature.set(tree.rootPlayer.kyokumen);
		{
			std::cout << "tree: resisterNode" << std::endl;
			SearchNode* n1 = new SearchNode(Move(8, 7, false));
			SearchNode* n2 = new SearchNode(Move(80, 79, false));
			bool b1 = tree.resisterLeafNode(n1);
			bool b2 = tree.resisterLeafNode(n1);
			bool b3 = tree.resisterLeafNode(n2);
			tree.excludeLeafNode(n1);
			bool b4 = tree.resisterLeafNode(n1);
			std::cout << "1:" << b1 << " 2:" << b2 << " 3:" << b3 << " 4:" << b4 << std::endl;
			assert(b1 && !b2 && b3 && b4);
			std::cout << "rn test: ok" << std::endl;
			tree.excludeLeafNode(n2);
			delete n1; delete n2;
		}
		SearchAgent ag(tree, 0);

		tree.lastRefRootByThread.assign(1, 0);
		tree.permitSearch();
		std::thread th(&SearchAgent::loop, &ag);
		std::this_thread::sleep_for(500ms);
		tree.prohibitSearch();
		ag.alive = false;
		th.join();
		tree.foutTree();
	}
#endif
}