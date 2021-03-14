#include "stdafx.h"
#include "kifu.h"
#include "learn_util.h"
#include "usi.h"
#include <filesystem>
#include <fstream>

void LearnKifu::save(const SearchTree& tree, GameResult result, const std::string& dir_path) {
	/* ファイル形式 規格
	ファイル名：<出力日時>.sstkifu
	ファイル内容・・・
	player sente/gote		:自身の手番
	startnum <number>		:自身が始めに指した手数
	winner sente/gote		:勝者の手番
	position startpos ...	:棋譜(sfen形式)
	score <movenum> <score> [child (<usimove>:<score>)*]:対象の手数に対する評価値（子ノードも出力可、複数行記述可能）
	*/
	//ファイル名
	std::filesystem::create_directories(dir_path);
	std::string filename = dir_path + "/" + LearnUtil::getDateString() + ".sstkifu";
	std::ofstream fs(filename);
	//自身の手番
	const auto startnum = tree.getFirstMoveNum();
	if (startnum % 2 == 0) fs << "player sente\n";
	else fs << "player gote\n";
	//始めに指した手数
	fs << "startnum " << startnum << "\n";
	//勝者の手番
	switch (result)	{
		case GameResult::SenteWin: fs << "winner sente\n";	break;
		case GameResult::GoteWin: fs << "winner gote\n"; break;
		default: fs << "winner draw\n"; break;
	}
	//棋譜
	const auto& history = tree.getHistory();
	fs << "position "; 
	if (tree.getStartKyokumen().isStartpos()) fs << "startpos ";
	else fs << tree.getStartKyokumen().toSfen();
	fs << "moves ";
	for (const auto& node : history) fs << node->move.toUSI() << " ";
	fs << "\n";
	//評価値
	for (unsigned long long i = 0; i < history.size();i++) {
		const auto& root = history[i];
		fs << "score " << i << " " << root->eval << "\n";
	}
}

void LearnKifu::load(const std::string& file_path) {
	std::ifstream fs(file_path);
	std::string str;
	while (std::getline(fs,str)) {
		const auto tokens = usi::split(str, ' ');
		if (tokens[0] == "player") {
			player_teban = (tokens[1] == "sente");
		}
		else if (tokens[0] == "startnum") {
			start = std::stoull(tokens[1]);
		}
		else if (tokens[0] == "winner") {
			if (tokens[1] == "sente") result = GameResult::SenteWin;
			else if (tokens[1] == "gote") result = GameResult::GoteWin;
			else result = GameResult::Draw;
		}
		else if (tokens[0] == "position") {
			startpos = Kyokumen(tokens);
			history = Move::usiToMoves(tokens);
		}
	}
}