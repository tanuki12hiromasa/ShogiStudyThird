#include "stdafx.h"
#include "leaf_guard.h"

std::mutex LeafGuard::mutex;
std::unordered_map<SearchNode*, bool> LeafGuard::nmap;

LeafGuard::LeafGuard(SearchNode* const node)
	:node(node)
{
	std::lock_guard<std::mutex> lock(mutex);
	if (node->isLeaf() && nmap.count(node) == 0) {//先約がいない場合
		nmap.insert(std::make_pair(node, true));
		result = true;
	}
	else { //先約がいる場合
		result = false;
	}
}

LeafGuard::~LeafGuard() {
	std::lock_guard<std::mutex> lock(mutex);
	if (result) {
		nmap.erase(node);
	}
}