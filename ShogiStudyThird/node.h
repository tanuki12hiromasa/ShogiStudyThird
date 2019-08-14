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


	std::vector<SearchNode*> children;
	Move move;
	bool isRep = false;
	std::atomic_bool isLeaf;
	std::atomic<double> eval;
	std::atomic<double> mass;

};