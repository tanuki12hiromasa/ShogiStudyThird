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
void Joseki::printOption() {
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
	std::string path = joseki_folder_name + "\\" + joseki_input_file_name;
	std::cout << path << "から定跡を読み込みます。" << std::endl;
	std::ifstream ifs(path);
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
			bn.bestMove = Move(column[0],true);
			//出現回数を0にしておく
			bn.appear = 0;

			bookJoseki.emplace(sfen, bn);
		}
	}
	std::cout << "読み込み完了。" << std::endl;
}

Joseki::bookNode Joseki::getBestMove(std::string sfen)
{
	bookNode bn;	//出現回数には-1が入っている
	if (bookJoseki.find(sfen) != bookJoseki.end()) {
		return bookJoseki[sfen];
	}
	bn.appear = -1;
	return bn;
}
