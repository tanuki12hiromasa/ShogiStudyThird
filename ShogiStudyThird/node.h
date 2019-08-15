#pragma once
#include "move.h"
#include <shared_mutex>
#include <atomic>
#include <vector>

class SearchNode {
public:
	SearchNode(const Move& move);
	SearchNode(const SearchNode&) = delete;
	SearchNode(SearchNode&&) = delete;

	SearchNode* addChild(const Move& move);

	SearchNode* choiceNode(const double pip, const double T)const;
	void updateNode(const double T);
	void updateMateNode();
	void setEvaluation(const double eval);

	std::vector<SearchNode*> children;
	Move move;
	bool isRep = false;
	std::atomic_bool isLeaf;
	std::atomic<double> eval;
	std::atomic<double> mass;

};