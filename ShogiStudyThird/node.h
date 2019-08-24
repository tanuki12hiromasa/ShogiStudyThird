#pragma once
#include "move.h"
#include <shared_mutex>
#include <atomic>
#include <vector>

class SearchNode {
private:
	static double repEval;
	static double T_depth;
	static double T_eval;
public:
	static double setRepEval(const double e) { repEval = e; }
	static double setT(const double T_d, const double T_e) { T_depth = T_d; T_eval = T_e; }
public:
	SearchNode(const Move& move);
	SearchNode(const SearchNode&) = delete;
	SearchNode(SearchNode&&) = delete;

	SearchNode* addChild(const Move& move);

	SearchNode* choiceNode(const double pip, const double T_choice)const;
	void updateNode();
	void updateMateNode();
	void setEvaluation(const double eval);
	double getEvaluation() { return isRep ? repEval : eval.load(); }

	void setMate();
	void setUchiFuMate();
	void setDeclare();

	std::vector<SearchNode*> children;
	Move move;
	bool isRep = false;
	std::atomic_bool isLeaf;
	std::atomic<double> eval;
	std::atomic<double> mass;

};

double SearchNode::repEval = 0;