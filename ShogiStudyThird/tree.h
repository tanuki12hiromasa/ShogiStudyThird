#pragma once
#include "node.h"
#include "kyokumen.h"
#include "evaluator.h"

class SearchTree {
public:
	SearchTree();
	void clear();
	
	void setT(double T_e, double T_d, std::vector<double> T_c);
	void proceed(SearchNode* node);
	void addNodenum(int n) { nodenum += n; }

	SearchNode* getRoot();
	const std::vector<SearchNode*>& getHistory()const { return history; }
	Kyokumen getRootKyokumen()const { return rootKyokumen; }
	double getTchoice();
	double getTeval() { return T_eval; }

private:
	SearchNode* rootNode;
	SearchNode* oldrootNode;
	Kyokumen rootKyokumen;
	std::vector<SearchNode*> history;
	Kyokumen startKyokumen;
	std::atomic_uint64_t nodenum;

	std::array<double, 64> T_choice;
	std::atomic_uint T_c_count;
	double T_eval;
	double T_depth;

	

#ifdef EVAL_DIFF_ONLY //差分計算を行う場合の
public:
	const Feature& getRootFeature()const { return rootFeature; }
private:
	Feature rootFeature;//root局面の特徴量
#endif

};