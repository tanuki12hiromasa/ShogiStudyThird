#pragma once
#include "tree.h"

class LearnKifu {
public:
	static void save(const SearchTree&, GameResult result, const std::string& dir_path = "./kifu");
public:
	LearnKifu(){}
	LearnKifu(const std::string& file_path) { load(file_path); }
	void load(const std::string& file_path);

	std::vector<Move> history;
	Kyokumen startpos;
	std::uint64_t start = 0;
	bool player_teban = true;
	GameResult result = GameResult::Draw;

};