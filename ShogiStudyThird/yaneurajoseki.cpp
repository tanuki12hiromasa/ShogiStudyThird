#include "yaneurajoseki.h"
#include "usi.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <Windows.h>
#include <climits>
#include <random>

YaneuraJoseki::YaneuraJoseki(){
	option.addOption("yjoseki_on", "check", "false");
	option.addOption("yjoseki_random", "check", "false");
	option.addOption("yjoseki_random", "check", "false");
	option.addOption("yjoseki_folder_name", "string", "joseki");
	option.addOption("yjoseki_input_name", "string", "joseki.bin");
}

//末尾の数字を取り除く
std::string YaneuraJoseki::getSfenTrimed(std::string sfen) {
	int i = sfen.length() - 1;
	while (sfen[i] == ' ') { --i; }
	while (isdigit(sfen[i])) { --i; }
	while (sfen[i] == ' ') { --i; }
	return sfen.substr(0, i);
}

//ひとまずstdを使用して実装
void YaneuraJoseki::readBook() {
	if (!option.getC("yjoseki_on")) {
		return;
	}
	std::string path = option.getS("yjoseki_folder_name") + "\\" + option.getS("yjoseki_input_name");
	std::cout << "input yjoseki from" << path << std::endl;
	std::ifstream ifs(path);
	std::string sfen;
	std::vector<bookNode>candidate;
	if (ifs.is_open()) {
		while (!ifs.eof()) {
			std::string line;
			std::getline(ifs, line);
			//sfenを見つけたら格納
			if (line.length() >= 4 && line.substr(0, 4) == "sfen") {
				if (candidate.size() > 0) {
					int select = 0;
					if (option.getC("yjoseki_random")) {
						std::random_device rnd;
						std::mt19937 mt(rnd());
						std::uniform_int_distribution<> randdis(0, candidate.size() - 1);

						select = randdis(mt);
					}
					bookJoseki.emplace(sfen, candidate[select]);
					candidate.clear();
				}
				sfen = getSfenTrimed(line);
			}
			else if (line.length() > 0 && line[0] != '#') {
				bookNode bn;
				auto column = usi::split(line, ' ');

				//最善手
				bn.bestMove = Move(column[0], true);
				//深さ
				bn.depth = std::stoi(column[3]);
				//仮に出現回数を0にしておく
				bn.on = true;

				for (int i = 0; i < bn.depth + 1; ++i) {
					candidate.push_back(bn);
				}
			}
		}
		std::cout << "読み込み完了。" << std::endl;
	}
	else {
		std::cout << "読み込みに失敗しました。定跡なしで開始します。" << std::endl;
	}
}

std::string YaneuraJoseki::getBestMoveFromJoseki(std::string sfen)
{
	//Softmax以外の定跡を読み込んであったら、それを利用する
	auto bestMove = getBestMove(YaneuraJoseki::getSfenTrimed(sfen));
	//appearが-1でなければ定跡があるので利用する
	if (bestMove.on) {
		auto bestChild = bestMove.bestMove;
		//std::cout << "bestmove " << bestChild.toUSI() << std::endl;
		return bestChild.toUSI();
	}
	else {
		//定跡が終わったので読み込みをオフに
		option.setOption("setoption name yjoseki_on value false");
		return "nullmove";
	}
}

void YaneuraJoseki::outputJosekiAsYaneura(SearchNode* node, Kyokumen kyokumen, std::string* st, int depth, int maxCount) {
	//if (node->children.size() == 0) {
	//	return;
	//}
	//if (node->move.toUSI() != "nullmove" && node->move.toUSI() != "1a1a") {
	//	kyokumen.proceed(node->move);
	//}
	//std::string sfen = kyokumen.toSfen();
	//std::vector<std::string> ts = usi::split(sfen, ' ');
	//ts[ts.size() - 1] = std::to_string(depth);
	////ts[ts.size() - 3] = depth % 2 == 0 ? "b" : "w";
	//sfen = "";
	//for (int i = 0; i < ts.size(); ++i) {
	//	if (i != 0) {
	//		sfen += " ";
	//	}
	//	sfen += ts[i];
	//}
	//*st += sfen + "\n";
	//int size = node->children.size();
	//int maxSize = maxCount;
	////int count = (node->children.size()/* > 0*/);
	//int count = (size > maxSize ? maxSize : size);
	//for (int i = 0; i < count; ++i) {
	//	SearchNode child = node->children[i];
	//	std::string move = child.move.toUSI();
	//	std::string next = "none";
	//	if (child.children.size() > 0) {
	//		next = child.children[0].move.toUSI();
	//	}
	//	std::string eval = std::to_string(int(child->eval + 0.5));
	//	std::string depth = std::to_string(int(child->mass + 0.5));
	//	std::string selected = "0";
	//	*st += move + " " + next + " " + eval + " " + depth + " " + selected + " " + "\n";
	//}

	//for (int i = 0; i < count/*node->children.size()*/; ++i) {
	//	SearchNode* child = node->children[i];
	//	outputJosekiAsYaneura(child, kyokumen, st, depth + 1, maxCount);
	//}
	////std::cout << *st << std::endl;
}

YaneuraJoseki::bookNode YaneuraJoseki::getBestMove(std::string sfen)
{
	bookNode bn;	//出現回数には-1が入っている
	if (bookJoseki.find(sfen) != bookJoseki.end()) {
		bn = bookJoseki[sfen];
		lastDepth = bn.depth;
	}
	else {
		bn.on = false;
		if (lastDepth >= 2) {
			std::cout << "outofbook" << std::endl;
		}
	}
	return bn;
}
