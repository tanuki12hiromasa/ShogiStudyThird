#include "joseki.h"
#include "usi.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <Windows.h>
#include <climits>

void Joseki::setOption(std::vector<std::string> tokens){
	auto t = tokens[2];
	if (t == "joseki_on") {
		joseki_on = (tokens[4] == "true");
	}
	else if (t == "joseki_folder_name") {
		joseki_folder_name = tokens[4];
	}
	else if (t == "joseki_input_name") {
		joseki_input_file_name = tokens[4];
	}
}
void Joseki::coutOption() {
	std::cout << "option name joseki_on type check default false" << std::endl;
	std::cout << "option name joseki_folder_name type string default joseki" << std::endl;
	std::cout << "option name joseki_input_file_name type string default user_book1.db" << std::endl;
}

//末尾の数字を取り除く
std::string Joseki::getSfenTrimed(std::string sfen) {
	int i = sfen.length() - 1;
	while (sfen[i] == ' ') { --i; }
	while (isdigit(sfen[i])) { --i; }
	while (sfen[i] == ' ') { --i; }
	return sfen.substr(0, i);
}

//ひとまずstdを使用して実装
void Joseki::readBook() {
	if (!joseki_on) {
		return;
	}
	std::string path = joseki_folder_name + "\\" + joseki_input_file_name;
	std::cout << path << "から定跡を読み込みます。" << std::endl;
	std::ifstream ifs(path);
	if (ifs.is_open()) {
		while (!ifs.eof()) {
			std::string line;
			std::getline(ifs, line);
			//sfenを見つけたら格納
			if (line.length() >= 4 && line.substr(0, 4) == "sfen") {
				bookNode bn;

				//末尾の数字を取り除く
				std::string sfen = getSfenTrimed(line);
				//次の行に最善手があるので読む
				std::getline(ifs, line);
				auto column = usi::split(line, ' ');

				//最善手
				bn.bestMove = Move(column[0], true);
				//出現回数を0にしておく
				bn.on = true;

				bookJoseki.emplace(sfen, bn);
			}
		}
		std::cout << "読み込み完了。" << std::endl;
	}
	else {
		std::cout << "読み込みに失敗しました。定跡なしで開始します。" << std::endl;
	}
}

bool Joseki::getBestMoveFromJoseki(std::string sfen)
{
	//Softmax以外の定跡を読み込んであったら、それを利用する
	auto bestMove = getBestMove(Joseki::getSfenTrimed(sfen));
	//appearが-1でなければ定跡があるので利用する
	if (bestMove.on) {
		auto bestChild = bestMove.bestMove;
		std::cout << "bestmove " << bestChild.toUSI() << std::endl;
		return true;
	}
	else {
		//定跡が終わったので読み込みをオフに
		joseki_on = false;
		return false;
	}
}

Joseki::bookNode Joseki::getBestMove(std::string sfen)
{
	bookNode bn;	//出現回数には-1が入っている
	if (bookJoseki.find(sfen) != bookJoseki.end()) {
		return bookJoseki[sfen];
	}
	bn.on = false;
	return bn;
}
