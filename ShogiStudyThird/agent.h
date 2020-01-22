#pragma once
#include "tree.h"
#include "move_gen.h"
#include <random>

class SearchAgent {
public:
	static void setFailnum(unsigned num) { maxfailnum = num; }
	static void setLeaveQSNode(bool b) { leave_QsearchNode = b; }
private:
	static unsigned maxfailnum;
	static bool leave_QsearchNode;
public:
	SearchAgent(SearchTree& tree, int seed);
	SearchAgent(SearchAgent&&)noexcept;
	SearchAgent() = delete;
	SearchAgent(const SearchAgent&) = delete;
	SearchAgent& operator=(const SearchAgent&) = delete;

	void loop();
	void stop() { alive = false; }
	void terminate() { alive = false; th.join(); }
private:
	size_t simulate(SearchNode* const root);
	size_t qsimulate(SearchNode* const root, const SearchPlayer& player);
	bool checkRepetitiveCheck(const Kyokumen& k,const std::vector<SearchNode*>& searchhis, const SearchNode* const latestRepnode)const;
	void nodeCopy(const SearchNode* const origin, SearchNode* const copy)const;
	SearchTree& tree;
	SearchNode* const root;
	SearchPlayer player;
	std::atomic_bool alive;//生きているかどうか
	std::thread th;

	//値域 [0,1.0) のランダムな値
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine; //初期シードはコンストラクタで受け取る

	friend class ShogiTest;
};