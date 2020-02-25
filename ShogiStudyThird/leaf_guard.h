#pragma once
#include "node.h"
#include <unordered_map>
#include <mutex>

class LeafGuard {
private:
	static std::mutex mutex;
public:
	LeafGuard(SearchNode* const);
	bool Result() const { return result; }
	~LeafGuard();
private:
	bool result;
	SearchNode* const node;
};