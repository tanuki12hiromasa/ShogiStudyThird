#pragma once
#include "tree.h"
#include <random>

class SearchAgent {
public:
	SearchAgent(SearchTree& tree, int seed);
	SearchAgent(SearchAgent&&)noexcept;
	SearchAgent() = delete;
	SearchAgent(const SearchAgent&) = delete;

	void loop();
	void simulate();

	bool immigrated();//setRootで設定されたrootからの探索になっているかどうか(前ターン時の探索が終わっていなければfalseを返す)
private:
	SearchTree& tree;
	std::atomic<SearchNode*> root;
public:
	std::atomic_bool enable = false;//探索をするかどうか
	std::atomic_bool alive = true;//生きているかどうか

private:
	//値域 [0,1.0) のランダムな値
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine; //初期シードはコンストラクタで受け取る
};