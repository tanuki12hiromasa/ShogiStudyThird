//#include "yaneurajoseki.h"
//#include "usi.h"
//#include <iostream>
//#include <fstream>
//#include <queue>
//#include <Windows.h>
//#include <climits>
//#include <random>
//
//void YaneuraJoseki::setOption(std::vector<std::string> tokens) {
//	auto t = tokens[2];
//	if (t == "yjoseki_on") {
//		joseki_on = (tokens[4] == "true");
//	}
//	else if (t == "yjoseki_random") {
//		joseki_random = (tokens[4] == "true");
//	}
//	else if (t == "yjoseki_folder_name") {
//		joseki_folder_name = tokens[4];
//	}
//	else if (t == "yjoseki_input_name") {
//		joseki_input_file_name = tokens[4];
//	}
//}
//void YaneuraJoseki::coutOption() {
//	std::cout << "option name yjoseki_on type check default false" << std::endl;
//	std::cout << "option name yjoseki_random type check default false" << std::endl;
//	std::cout << "option name yjoseki_folder_name type string default joseki" << std::endl;
//	std::cout << "option name yjoseki_input_file_name type string default user_book1.db" << std::endl;
//}
//
////末尾の数字を取り除く
//std::string YaneuraJoseki::getSfenTrimed(std::string sfen) {
//	int i = sfen.length() - 1;
//	while (sfen[i] == ' ') { --i; }
//	while (isdigit(sfen[i])) { --i; }
//	while (sfen[i] == ' ') { --i; }
//	return sfen.substr(0, i);
//}
//
////ひとまずstdを使用して実装
//void YaneuraJoseki::readBook() {
//	if (!joseki_on) {
//		return;
//	}
//	std::string path = joseki_folder_name + "\\" + joseki_input_file_name;
//	std::cout << path << "から定跡を読み込みます。" << std::endl;
//	std::ifstream ifs(path);
//	std::string sfen;
//	std::vector<bookNode>candidate;
//	if (ifs.is_open()) {
//		while (!ifs.eof()) {
//			std::string line;
//			std::getline(ifs, line);
//			//sfenを見つけたら格納
//			if (line.length() >= 4 && line.substr(0, 4) == "sfen") {
//				if (candidate.size() > 0) {
//					int select = 0;
//					if (joseki_random) {
//						std::random_device rnd;
//						std::mt19937 mt(rnd());
//						std::uniform_int_distribution<> randdis(0, candidate.size() - 1);
//
//						select = randdis(mt);
//					}
//					bookJoseki.emplace(sfen, candidate[select]);
//					candidate.clear();
//				}
//				sfen = getSfenTrimed(line);
//			}
//			else if (line.length() > 0 && line[0] != '#') {
//				bookNode bn;
//				auto column = usi::split(line, ' ');
//
//				//最善手
//				bn.bestMove = Move(column[0], true);
//				//深さ
//				bn.depth = std::stoi(column[3]);
//				//仮に出現回数を0にしておく
//				bn.on = true;
//
//				for (int i = 0; i < bn.depth + 1; ++i) {
//					candidate.push_back(bn);
//				}
//			}
//		}
//		std::cout << "読み込み完了。" << std::endl;
//	}
//	else {
//		std::cout << "読み込みに失敗しました。定跡なしで開始します。" << std::endl;
//	}
//}
//
//std::string YaneuraJoseki::getBestMoveFromJoseki(std::string sfen)
//{
//	//Softmax以外の定跡を読み込んであったら、それを利用する
//	auto bestMove = getBestMove(YaneuraJoseki::getSfenTrimed(sfen));
//	//appearが-1でなければ定跡があるので利用する
//	if (bestMove.on) {
//		auto bestChild = bestMove.bestMove;
//		//std::cout << "bestmove " << bestChild.toUSI() << std::endl;
//		return bestChild.toUSI();
//	}
//	else {
//		//定跡が終わったので読み込みをオフに
//		joseki_on = false;
//		return "nullmove";
//	}
//}
//
//YaneuraJoseki::bookNode YaneuraJoseki::getBestMove(std::string sfen)
//{
//	bookNode bn;	//出現回数には-1が入っている
//	if (bookJoseki.find(sfen) != bookJoseki.end()) {
//		bn = bookJoseki[sfen];
//		lastDepth = bn.depth;
//	}
//	else {
//		bn.on = false;
//		if (lastDepth >= 2) {
//			std::cout << "outofbook" << std::endl;
//		}
//	}
//	return bn;
//}
