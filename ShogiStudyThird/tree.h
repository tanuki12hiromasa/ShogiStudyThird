#pragma once
#include "node.h"
#include "kyokumen.h"
#include "evaluator.h"
#include "player.h"

class SearchTree {
public:
	SearchTree();
	void clear();
	
	void setT(double T_e, double T_d, std::vector<double> T_c);
	void proceed(SearchNode* node);
	void addNodenum(int n) { nodenum += n; }

	SearchNode* getRoot();
	const std::vector<SearchNode*>& getHistory()const { return history; }
	const SearchPlayer& getRootPlayer()const { return rootPlayer; }
	double getTchoice();
	double getTeval() { return T_eval; }

private:
	SearchNode* rootNode;
	SearchNode* oldrootNode;
	SearchPlayer rootPlayer;
	std::vector<SearchNode*> history;
	Kyokumen startKyokumen;
	std::atomic_uint64_t nodenum;

	std::array<double, 64> T_choice;
	std::atomic_uint T_c_count;
	double T_eval;
	double T_depth;
};