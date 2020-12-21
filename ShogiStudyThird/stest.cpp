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

bool ShogiTest::checkStringsInclusion(strv u, strv a) {
	for (auto& str : a) {
		auto result = std::find(u.begin(), u.end(), str);
		if (result == u.end()) return false;
		else u.erase(result);
	}
	return true;
}

void ShogiTest::coutStringsDiff(const strv& cor,const strv& s) {
	strv cc = cor;
	strv sc = s;
	for (auto& str : cor) {
		auto result = std::find(sc.begin(), sc.end(), str);
		if(result!=sc.end())sc.erase(result);
	}
	std::cout<<"yobun : ";
	for (const auto& s : sc)std::cout << s << ' ';
	std::cout << std::endl;
	for (auto& str : s) {
		auto result = std::find(cc.begin(), cc.end(), str);
		if (result != cc.end())cc.erase(result);
	}
	std::cout << "fusoku : ";
	for (const auto& s : cc)std::cout << s << ' ';
	std::cout << std::endl;
}

bool ShogiTest::genMoveCheck(std::string parent_sfen, std::string child_moves) {
	strv ans = usi::split(child_moves, ' ');
	Kyokumen k(usi::split(parent_sfen, ' '));
	Move nmove;
	const auto moves = MoveGenerator::genAllMove(nmove, k);
	strv msv; for (const auto& m : moves)msv.push_back(m.toUSI());
	if (checkStrings(ans, msv)) return true;
	else {
		std::cout << k.toBanFigure() << std::endl;
		coutStringsDiff(ans, msv);
		assert(0);
		return false;
	} 
}

bool ShogiTest::genMoveCheck(std::string parent_sfen, Move pmove, std::string child_moves) {
	strv ans = usi::split(child_moves, ' ');
	Kyokumen k(usi::split(parent_sfen, ' '));
	Move nmove;
	const auto moves = MoveGenerator::genAllMove(nmove, k);
	strv msv; for (const auto& m : moves)msv.push_back(m.toUSI());
	if (checkStrings(ans, msv)) return true;
	else {
		std::cout << k.toBanFigure() << std::endl;
		coutStringsDiff(ans, msv);
		assert(0);
		return false;
	}
}

bool ShogiTest::genCapMoveCheck(std::string parent_sfen) {
#if 0
	//合法手=capmove+nocapmoveを確かめる
	Kyokumen k(usi::split(parent_sfen, ' '));
	Move m;
	SearchNode* const root = new SearchNode(m);
	MoveGenerator::genMove(root, k);
	const auto& moves = root->children;
	strv msv; for (const auto& m : moves)msv.push_back(m->move.toUSI());
	const auto cmoves = MoveGenerator::genCapMove(m, k);
	strv cmsv; for (const auto& m : cmoves)cmsv.push_back(m.toUSI());
	const auto nmoves = MoveGenerator::genNocapMove(m, k);
	strv nmsv; for (const auto& m : nmoves)nmsv.push_back(m.toUSI());

	if (checkStringsUnion(msv, cmsv, nmsv)) { 
		std::cout << "gencapmove test ok" << std::endl;
		return true; 
	}
	else {
		std::cout << k.toBanFigure() << std::endl;
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
#endif
	//合法手⊃capmoveを確かめる
	Kyokumen k(usi::split(parent_sfen, ' '));
	Move m;
	const auto moves = MoveGenerator::genMove(m, k);
	strv msv; for (const auto& m : moves)msv.push_back(m.toUSI());
	const auto cmoves = MoveGenerator::genCapMove(m, k);
	strv cmsv; for (const auto& m : cmoves)cmsv.push_back(m.toUSI());

	if (checkStringsInclusion(msv, cmsv)) {
		std::cout << "genfull: ";
		for (const auto& s : msv)std::cout << s << ' ';
		std::cout << std::endl;
		std::cout << "gencap: ";
		for (const auto& s : cmsv)std::cout << s << ' ';
		std::cout << std::endl;
		std::cout << "gencapmove test ok" << std::endl;
		return true;
	}
	else {
		std::cout << k.toBanFigure() << std::endl;
		std::cout << "genfull: ";
		for (const auto& s : msv)std::cout << s << ' ';
		std::cout << std::endl;
		std::cout << "gencap: ";
		for (const auto& s : cmsv)std::cout << s << ' ';
		std::cout << std::endl;
		assert(0);
		return false;
	}
}

bool ShogiTest::checkFeature(std::string usiposition) {
	auto token = usi::split(usiposition, ' ');
	const auto moves = Move::usiToMoves(token);
	std::vector<std::string> startpos;
	for (const auto& str : token) {
		if (str == "moves") break;
		startpos.push_back(str);
	}
	Kyokumen kyokumen(startpos);
	Feature feat(kyokumen);
	for (int t = 0; t < moves.size(); t++) {
		const auto move = moves[t];
		feat.proceed(kyokumen, move);
		kyokumen.proceed(move);
		Feature stfeat(kyokumen);
		if (feat != stfeat) {
			std::cout << "feature test ng" << std::endl;
			std::cout << feat.toString() << std::endl;
			std::cout << stfeat.toString() << std::endl;
			assert(0);
			return false;
		}
	}
	std::cout << "feature test ok" << std::endl;
	return true;
}

bool ShogiTest::checkRecede(std::string sfen,const int depth) {
	Kyokumen k(usi::split(sfen,' '));
	Feature f(k);
	SearchNode* const root = new SearchNode(Move());
	const Kyokumen ck = k;
	const Feature cf = f;
	bool result = checkRecedeR(k, f, root, depth);
	if (result) {
		std::cout << "recede test ok" << std::endl;
		root->deleteTree();
		delete root;
		return true;
	}
	else {
		std::cout << "recede test ng" << std::endl;
		root->deleteTree();
		delete root;
		return false;
	}
}

bool ShogiTest::checkRecedeR(Kyokumen& k, Feature& f, SearchNode* p, const int depth) {
	const auto moves = MoveGenerator::genMove(p->move, k);
	p->children.reserve(moves.size());
	for (const auto move : moves) {
		p->addChild(move);
	}
	const Kyokumen ck = k;
	const Feature cf = f;
	for (const auto& c : p->children) {
		const auto cache = f.getCache();
		f.proceed(k, c->move);
		const auto cap = k.proceed(c->move);
		if (depth > 0) {
			bool result = checkRecedeR(k, f, c, depth - 1);
			if (!result) {
				return false;
			}
		}
		const auto moved = k.recede(c->move, cap);
		if (k != ck && k.eachKomaBB == ck.eachKomaBB) {
			std::cout << "error: kyokumen recede" << std::endl;
			std::cout << "correct:\n" << ck.toBanFigure() << std::endl;
			std::cout << "kyokumen:\n" << k.toBanFigure() << std::endl;
			assert(k == ck);
			return false;
		}
		f.recede(k, moved, cap, c->move, cache);
#if 0
		if (f != cf) { //apery_kppt
			std::cout << "error: feature recede" << std::endl;
			std::cout << "kyokumen:\nposition " << k.toSfen() << "\n" << k.toBanFigure() << std::endl;
			std::cout << "move: " << c->move.toUSI() << std::endl;
			std::cout << "feature diff:" << std::endl;
			for (int i = 0; i < 38; i++) {
				if (f.idlist.list0[i] != cf.idlist.list0[i]) {
					std::cout << "list0[" << i << "] " << f.idlist.list0[i] << " " << cf.idlist.list0[i] << std::endl;
				}
				if (f.idlist.list1[i] != cf.idlist.list1[i]) {
					std::cout << "list1[" << i << "] " << f.idlist.list1[i] << " " << cf.idlist.list1[i] << std::endl;
				}
			}
			if (f.idlist.material != cf.idlist.material) {
				std::cout << "material " << f.idlist.material << " " << cf.idlist.material << std::endl;
			}
			assert(f == cf);
			return false;
		}
#endif
	}
	return true;
}

void ShogiTest::test() {
	using namespace std;
	std::cout << "now initializing..." << std::endl;
	BBkiki::init();
	Evaluator::init();
	std::cout << "initialized." << std::endl;
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
		string str17 = "position sfen k8/6g1p/2gppspnP/1lbr+P1Gp1/2G3n2/2l6/9/9/K8 b rb3s2n2l11p 1";
		ShogiTest::genCapMoveCheck(str17);
		string str16 = "position sfen 1r3g3/l2gk3l/3p4p/2p1+B1p2/p4s3/2P1P1Pb1/1+p1PSP2P/L2S3GL/1NGKNsr2 b N3Pn3p 1";
		string moves16 = "P*2b P*2c P*2d P*2e P*2g P*2i P*8b P*8c P*8d P*8e P*8f P*8h P*9c P*9d P*9f P*9g P*9i N*1d N*1e N*1f N*1i N*2c N*2d N*2e N*2g N*2i N*3c N*3e N*3g N*3h N*4c N*4d N*4f N*4h N*5c N*5e N*5h N*6d N*6e N*6f N*7c N*7e N*7g N*7h N*8c N*8d N*8e N*8f N*8h N*9c N*9d N*9f N*9g N*9i 1g1f 3f3e 4g4f 5f5e 6g6f 7f7e 8i7g 8i9g 5g4f 5g4h 5g6f 6h7g 2h2g 2h2i 2h3g 2h3h 7i7h 7i8h 5d4d 5d5c 5d5e 5d6d 9h9e 9h9f 9h9g 5d2a 5d3b 5d4c 5d4e 5d6c 5d6e";
		ShogiTest::genMoveCheck(str16, moves16);
		ShogiTest::genCapMoveCheck(str16);
		string str15 = "position sfen 1r3g3/l2gk3l/3p4p/2p1+B1p2/p4s3/2P1P1Pb1/1+p1PSP2P/L2S3GL/1NGKNsr2 w N3Pn3p 1";
		string moves15 = "P*2a P*2b P*2c P*2d P*2e P*2g P*4b P*4c P*4d P*4f P*4h P*5a P*5c P*5e P*5h P*8b P*8c P*8d P*8e P*8f P*8h N*1a N*1d N*1e N*1f N*2a N*2b N*2c N*2d N*2e N*2g N*3a N*3b N*3c N*3e N*3g N*4b N*4c N*4d N*4f N*5a N*5c N*5e N*6a N*6d N*6e N*6f N*7a N*7b N*7c N*7e N*7g N*8b N*8c N*8d N*8e N*8f N*9a N*9c N*9d N*9f N*9g 1c1d 3d3e 6c6d 7d7e 9e9f 4e3f 4e4f 4e5d 4e5f 4i3h 4i3h+ 4i5h 4i5h+ 4a3a 4a3b 4a4b 4a5a 6b5c 6b6a 6b7b 6b7c 8g7g 8g7h 8g8f 8g8h 8g9g 8g9h 5b4b 5b5a 5b6a 9b9c 9b9d 2f1e 2f1g 2f1g+ 2f3e 2f3g 2f3g+ 2f4d 2f4h 2f4h+ 2f5c 2f5i 2f5i+ 3i1i 3i1i+ 3i2i 3i2i+ 3i3f 3i3f+ 3i3g 3i3g+ 3i3h 3i3h+ 8a5a 8a6a 8a7a 8a8b 8a8c 8a8d 8a8e 8a8f 8a9a";
		ShogiTest::genMoveCheck(str15, moves15);
		ShogiTest::genMoveCheck(str15, Move(9, 39, false), moves15);
		ShogiTest::genCapMoveCheck(str15);
		string str14 = "position sfen ln1gk1gnl/2s6/p1p1ppppp/3p3R1/4B4/2P6/P2PPPP1P/L3K4/1+rSG1GSNL b BSPn2p 1";
		string moves14 = "P*2b P*2e P*2f P*2g P*2h P*8b P*8c P*8d P*8e P*8f P*8g P*8h S*1b S*1d S*1e S*1f S*1h S*2b S*2e S*2f S*2g S*2h S*3b S*3d S*3e S*3f S*3h S*4a S*4b S*4d S*4e S*4f S*4h S*5b S*5d S*5f S*5i S*6b S*6c S*6e S*6f S*6h S*7a S*7d S*7e S*7g S*7h S*8b S*8c S*8d S*8e S*8f S*8g S*8h S*9b S*9d S*9e S*9f S*9i B*1b B*1d B*1e B*1f B*1h B*2b B*2e B*2f B*2g B*2h B*3b B*3d B*3e B*3f B*3h B*4a B*4b B*4d B*4e B*4f B*4h B*5b B*5d B*5f B*5i B*6b B*6c B*6e B*6f B*6h B*7a B*7d B*7e B*7g B*7h B*8b B*8c B*8d B*8e B*8f B*8g B*8h B*9b B*9d B*9e B*9f B*9i 1g1f 3g3f 4g4f 5g5f 6g6f 7f7e 9g9f 3i2h 3i3h 3i4h 7i6h 7i7h 7i8h 4i3h 4i4h 4i5i 6i5i 6i6h 6i7h 5h4h 5h5i 5h6h 1i1h 5e3c 5e3c+ 5e4d 5e4f 5e6d 5e6f 5e7g 5e8h 5e9i 2d1d 2d2c 2d2c+ 2d2e 2d2f 2d2g 2d2h 2d3d 2d4d 2d5d 2d6d";
		ShogiTest::genMoveCheck(str14, moves14);
		ShogiTest::genMoveCheck(str14, Move(27, 18, false), moves14);
		ShogiTest::genCapMoveCheck(str14);
		string str13 = "position sfen 1n1g1kgnl/l1s4p1/p1p1BppRp/9/9/2P6/P2PPPP1P/1P3K3/1NG2GSNL b RS3Pbslp 1";
		string moves13 = "P*2d P*2e P*2f P*2g P*2h S*1b S*1d S*1e S*1f S*1h S*2d S*2e S*2f S*2g S*2h S*3b S*3d S*3e S*3f S*3h S*4b S*4d S*4e S*4f S*5a S*5b S*5d S*5e S*5f S*5h S*5i S*6b S*6c S*6d S*6e S*6f S*6h S*6i S*7a S*7d S*7e S*7g S*7h S*8b S*8c S*8d S*8e S*8f S*8g S*9a S*9d S*9e S*9f S*9h S*9i R*1b R*1d R*1e R*1f R*1h R*2d R*2e R*2f R*2g R*2h R*3b R*3d R*3e R*3f R*3h R*4b R*4d R*4e R*4f R*5a R*5b R*5d R*5e R*5f R*5h R*5i R*6b R*6c R*6d R*6e R*6f R*6h R*6i R*7a R*7d R*7e R*7g R*7h R*8b R*8c R*8d R*8e R*8f R*8g R*9a R*9d R*9e R*9f R*9h R*9i 1g1f 3g3f 4g4f 5g5f 6g6f 7f7e 8h8g 9g9f 8i7g 3i2h 3i3h 4i3h 4i5h 4i5i 7i6h 7i6i 7i7h 4h3h 4h5h 4h5i 1i1h 5c2f 5c2f+ 5c3a 5c3a+ 5c3e 5c3e+ 5c4b 5c4b+ 5c4d 5c4d+ 5c6b 5c6b+ 5c6d 5c6d+ 5c7a 5c7a+ 5c7e 5c7e+ 5c8f 5c8f+ 2c1c 2c1c+ 2c2b 2c2b+ 2c2d 2c2d+ 2c2e 2c2e+ 2c2f 2c2f+ 2c2g 2c2g+ 2c2h 2c2h+ 2c3c 2c3c+";
		ShogiTest::genMoveCheck(str13, moves13);
		ShogiTest::genMoveCheck(str13, Move(88, 10, false), moves13);
		ShogiTest::genCapMoveCheck(str13);
		string str12 = "position sfen lns5k/8+P/ppp1p1nG1/6P2/3p2bp1/2P6/PPKP5/1gSlP4/LNb1+r3+l w RG2SNPg4p 1";
		string moves12 = "";
		ShogiTest::genMoveCheck(str12, moves12);
		ShogiTest::genMoveCheck(str12, Move(2, 1, true), moves12);
		ShogiTest::genCapMoveCheck(str12);
		string str11 = "position sfen lns5k/8r/ppp1p1nGP/6P2/3p2bp1/2P6/PPKP5/1gSlP4/LNb1+r3+l b G2SNPg4p 1";
		string moves11 = "P*2b P*2d P*2f P*2g P*2h P*2i P*4b P*4c P*4d P*4e P*4f P*4g P*4h P*4i N*1d N*1e N*1f N*1g N*1h N*2d N*2f N*2g N*2h N*2i N*3f N*3g N*3h N*3i N*4c N*4d N*4e N*4f N*4g N*4h N*4i N*5d N*5e N*5f N*5g N*6c N*6d N*6f N*6i N*7d N*7e N*8d N*8e N*8f N*9d N*9e N*9f N*9h S*1d S*1e S*1f S*1g S*1h S*2a S*2b S*2d S*2f S*2g S*2h S*2i S*3a S*3b S*3f S*3g S*3h S*3i S*4a S*4b S*4c S*4d S*4e S*4f S*4g S*4h S*4i S*5a S*5b S*5d S*5e S*5f S*5g S*6a S*6b S*6c S*6d S*6f S*6i S*7b S*7d S*7e S*8b S*8d S*8e S*8f S*9b S*9d S*9e S*9f S*9h G*1d G*1e G*1f G*1g G*1h G*2a G*2b G*2d G*2f G*2g G*2h G*2i G*3a G*3b G*3f G*3g G*3h G*3i G*4a G*4b G*4c G*4d G*4e G*4f G*4g G*4h G*4i G*5a G*5b G*5d G*5e G*5f G*5g G*6a G*6b G*6c G*6d G*6f G*6i G*7b G*7d G*7e G*8b G*8d G*8e G*8f G*9b G*9d G*9e G*9f G*9h 1c1b 1c1b+ 3d3c 3d3c+ 5h5g 6g6f 7f7e 8g8f 9g9f 7h6i 2c1b 2c2b 2c2d 2c3b 2c3c 7g8f 9i9h";
		ShogiTest::genMoveCheck(str11, moves11);
		ShogiTest::genMoveCheck(str11, Move(64, 1, false), moves11);
		ShogiTest::genCapMoveCheck(str11);
		string str10 = "position sfen l3k3l/9/3gp2p1/p1pB1PB1P/5N3/2n1S4/rPGPP4/KS3+r2+p/LN7 b GSNLPgs7p 1";
		string moves10 = "8i9g 6d9g 9h9g";
		ShogiTest::genMoveCheck(str10, moves10);
		ShogiTest::genMoveCheck(str10, Move(77, 78, false), moves10);
		ShogiTest::genCapMoveCheck(str10);
		string str9 = "position sfen 7nl/+P2+R3sk/4ppgpR/4s1N2/3n5/p1bp1bP1P/1P2PG3/9/L3K3L w GS7Pgsnlp 1";
		string moves9 = "2a1c 4f1c 1b1c";
		ShogiTest::genMoveCheck(str9, moves9);
		ShogiTest::genMoveCheck(str9, Move(3, 2, false), moves9);
		ShogiTest::genCapMoveCheck(str9);
		string str8 = "position sfen 6+Rnl/+P2g4k/4ppgpp/4s1N1R/3nb4/p1Gp1PP1P/1P2PG3/4K4/L7L w S6Pb2snl 1";
		string moves8 = "L*1e L*1g L*1h L*2b L*2d L*2e L*2f L*2g L*2h L*3b L*3e L*3g L*3h L*4a L*4b L*4d L*4e L*4h L*5a L*5b L*5f L*6a L*6c L*6d L*6g L*6h L*7a L*7b L*7c L*7d L*7e L*7g L*7h L*8a L*8b L*8c L*8d L*8e L*8f L*8h L*9a L*9c L*9d L*9e L*9g L*9h N*1e N*1g N*2b N*2d N*2e N*2f N*2g N*3b N*3e N*3g N*4a N*4b N*4d N*4e N*5a N*5b N*5f N*6a N*6c N*6d N*6g N*7a N*7b N*7c N*7d N*7e N*7g N*8a N*8b N*8c N*8d N*8e N*8f N*9a N*9c N*9d N*9e N*9g S*1e S*1g S*1h S*2b S*2d S*2e S*2f S*2g S*2h S*2i S*3b S*3e S*3g S*3h S*3i S*4a S*4b S*4d S*4e S*4h S*4i S*5a S*5b S*5f S*5i S*6a S*6c S*6d S*6g S*6h S*6i S*7a S*7b S*7c S*7d S*7e S*7g S*7h S*7i S*8a S*8b S*8c S*8d S*8e S*8f S*8h S*8i S*9a S*9c S*9d S*9e S*9g S*9h B*1e B*1g B*1h B*2b B*2d B*2e B*2f B*2g B*2h B*2i B*3b B*3e B*3g B*3h B*3i B*4a B*4b B*4d B*4e B*4h B*4i B*5a B*5b B*5f B*5i B*6a B*6c B*6d B*6g B*6h B*6i B*7a B*7b B*7c B*7d B*7e B*7g B*7h B*7i B*8a B*8b B*8c B*8d B*8e B*8f B*8h B*8i B*9a B*9c B*9d B*9e B*9g B*9h 1c1d 2c2d 4c4d 6f6g 6f6g+ 9f9g 9f9g+ 6e5g 6e5g+ 6e7g 6e7g+ 5d4e 5d6c 3c2d 3c3b 3c3d 3c4d 6b5b 6b6a 6b6c 6b7b 6b7c 5e4d 5e4f 5e6d 5e7c 5e8b 5e9a";
		ShogiTest::genMoveCheck(str8, moves8);
		ShogiTest::genMoveCheck(str8, Move(44, 43, false), moves8);
		ShogiTest::genCapMoveCheck(str8);
		string str7 = "position sfen lnsgk1snl/1r4gb1/ppppppBpp/9/9/2P6/PP1PPPPPP/7R1/LNSGKGSNL w P 1";
		string moves7 = "2a3c 3a4b 3b3c 3b4b 2b3c 8b4b 5a4a 5a5b 5a6b";
		ShogiTest::genMoveCheck(str7, moves7);
		ShogiTest::genMoveCheck(str7, Move(70, 20, false), moves7);
		string str6 = "position sfen lnsg1g1nl/1r2k2s1/1ppppp1pp/p8/9/2P6/PPbPPPPPP/1R7/LNSGKGSNL b BP 1";
		string moves6 = "8i7g 7i6h 6i6h 8h6h B*6h 5i4h 5i5h";
		ShogiTest::genMoveCheck(str6, Move(92, 60, false), moves6);
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
	{
		string str4 = "position startpos moves 2g2f 8c8d 7g7f 8d8e 8h7g 3c3d 7i8h 2b7g+ 8h7g 3a2b 2f2e 2b3c 3i3h 5a4b 5i6h 7a7b 4g4f 7b8c 3g3f 8c8d 2i3g 4a3b 3f3e 3d3e 3g4e 3c2b 2e2d 6a5b B*6f B*4d 6i7h 3e3f 9g9f 4d6f 7g6f 4c4d";
		ShogiTest::checkFeature(str4);
		string str3 = "position startpos moves 2g2f 3c3d 7g7f 4c4d 5i6h 3a3b 4g4f 8b4b 3i4h 5a6b 9g9f 6b7b 9f9e 7b8b 4h4g 9a9b 4g5f 8b9a 8h6f 3b4c 4i5h 6c6d 2f2e 2b3c 6h7h 4a5b 4f4e 7a8b 6i6h 6a7a 6f7g 5c5d 2h2g 5b5c 3g3f 5d5e 2e2d 2c2d 5f5e 4d4e 2i3g 4c5b P*4d P*5d 3g4e 5d5e 4e5c+ 5b5c G*4c N*6e 7g6f 5e5f 5g5f P*5g 5h5i S*3h 2g2h 3h4g 4c5c 4b4d S*6b 5g5h+ 7h8h 5h6h 5i6h 4g5f+ 6b7a+ 8b7a G*6b 7a8b P*4e 4d4e 6f3c+ 2a3c 6g6f B*3i B*6c 3i2h+ 6b7b G*7a 5c6b 7a7b 6b7b S*7g 8i7g 6e7g+ 6h7g 4e4h+ G*7h G*7a N*7e N*8e 7b7a 8e7g+ 8h9g 8b7a 7e8c 9a8b 8c7a+ G*8f 8g8f G*8g 7h8g 7g8g 9g8g R*8i S*8h 4h5g G*7g 5g7g 8g7g 8i7i+ 8h7i 5f6f 7g6f G*6e 6f6g 6e5f 6g7h";
		ShogiTest::checkFeature(str3);
		string str2 = "position startpos moves 7g7f 3c3d 8h2b 3a2b 7i8h 2b3c 2g2f 8b2b 5i6h 5a6b B*6e 7c7d 6e4c+ 6c6d 6h7h 4a5b 4c5b 6a5b 9g9f 9c9d 4i5h 6b7b 1g1f 5b6c 5h6h 7b8b 8h7g 8c8d 3i4h 5c5d 1f1e 7a7b 3g3f 3c4d 4g4f 3d3e 6i7i 3e3f 4h4g 4d3e 4g5f 3e4f 2h3h B*2g 3h4h P*4g 4h4g 4f4g+ 5f4g R*4h 4g5f 2g3h+ P*4g 3h2i G*5h 4h4i+ 7i6i 2i1i 1e1d 1c1d 9f9e 9d9e 7h8h 3f3g+ 8h7h L*6a";
		ShogiTest::checkFeature(str2);
		string str1 = "position startpos moves 7g7f 3c3d 8h2b+ 3a2b 7i8h 2b3c 2g2f 8b2b 5i6h 5a6b B*6e 7c7d 6e4c+ 6c6d 6h7h 4a5b 4c5b 6a5b 9g9f 9c9d 4i5h 6b7b 1g1f 5b6c 5h6h 7b8b 8h7g 8c8d 3i4h 5c5d 1f1e 7a7b 3g3f 3c4d 4g4f 3d3e 6i7i 3e3f 4h4g 4d3e 4g5f 3e4f 2h3h B*2g 3h4h P*4g 4h4g 4f4g+ 5f4g R*4h 4g5f 2g3h+ P*4g 3h2i G*5h 4h4i+ 7i6i 2i1i 1e1d 1c1d 9f9e 9d9e 7h8h 3f3g+ 8h7h L*6a";
		ShogiTest::checkFeature(str1);
	}
	{
		string str5 = "position sfen ln5nl/1r2gkgs1/p1ppp2pp/1s3p1P1/1p3N3/P1PS1Pp2/1P1PP3P/2GK2SR1/LN3G2L b Bbp 1";
		checkRecede(str5, 1);
		string str4 = "position sfen 1r3g3/l2gk3l/3p4p/2p1+B1p2/p4s3/2P1P1Pb1/1+p1PSP2P/L2S3GL/1NGKNsr2 b N3Pn3p 1";
		checkRecede(str4, 1);
		string str3 = "position sfen ln1gk1gnl/2s6/p1p1ppppp/3p3R1/4B4/2P5P/P2PPPP2/L3K4/1+rSG1GSNL w BSPn2p 1";
		checkRecede(str3, 0);
		string str2 = "position sfen ln1gk1gnl/2s6/p1p1ppppp/3p3R1/4B4/2P6/P2PPPP1P/L3K4/1+rSG1GSNL b BSPn2p 1";
		checkRecede(str2, 1);
		string str1 = "position sfen l3k2nl/2g3gb1/3psppp1/prp5p/9/2P1R3P/PP1PPPP2/2G3S2/L1S1KG1NL b S2Pb2np 1";
		checkRecede(str1, 1);
	}
#if 0
	{
		Commander com;
		std::cout << "initializing now..." << std::endl;
		com.setOption(usi::split("setoption name USI_Ponder value false", ' '));
		com.setOption(usi::split("setoption name leave_branchNode value true", ' '));
		com.setOption(usi::split("setoption name leave_qsearchNode value true", ' '));
		com.setOption(usi::split("setoption name NumOfAgent value 6", ' '));
		com.gameInit();
		com.tree.set(usi::split("position startpos moves 7g7f 8c8d 9g9f 8d8e 7i7h 8e8f 8g8f 8b8f 8h9g 8f9f 9g8h 9f8f 8h9g 8f9f 9g8h 9f8f 8h9g 8f9f 9g8h 9f8f 8h9g", ' '));
		std::cout << "search start(500ms)" << std::endl;
		com.go(usi::split("go btime 0 wtime 0 byoyomi 650", ' '));
		std::this_thread::sleep_for(510ms);
		com.tree.prohibitSearch();
		com.tree.foutTree();
	}
#endif
}