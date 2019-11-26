#pragma once
#include "node.h"
#include <unordered_map>
#include <mutex>

class LeafGuard {
private:
	static std::mutex mutex;
	static std::unordered_map<SearchNode*, bool> nmap;
public:
	LeafGuard(SearchNode* const);
	bool Result() const { return result; }
	~LeafGuard();
private:
	bool result;
	SearchNode* const node;
};