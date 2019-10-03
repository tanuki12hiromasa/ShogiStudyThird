#pragma once
#include "tree.h"
#include <random>

class SearchAgent {
public:
	SearchAgent(SearchTree& tree, unsigned threadid, int seed);
	SearchAgent(SearchTree& tree, unsigned threadid) :SearchAgent(tree, threadid, threadid){}
	SearchAgent(SearchAgent&&)noexcept;
	SearchAgent() = delete;
	SearchAgent(const SearchAgent&) = delete;

	void loop();
	void terminate() { alive = false; }
private:
	size_t simulate(SearchNode* const root);

	SearchTree& tree;
	unsigned ID;
	std::atomic_bool alive;//生きているかどうか

	//値域 [0,1.0) のランダムな値
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine; //初期シードはコンストラクタで受け取る
};