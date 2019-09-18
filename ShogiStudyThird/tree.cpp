#include "stdafx.h"
#include "tree.h"

bool SearchTree::resisterLeafNode(SearchNode* const node) {
	std::lock_guard<std::mutex> lock(lnmutex);
	if (nmap.count(node) == 0) {//先約がいない場合
		nmap.insert(std::make_pair(node, 1));
		return true;
	}
	else { //先約がいる場合
		return false;
	}
}

void SearchTree::excludeLeafNode(SearchNode* const node) {
	std::lock_guard<std::mutex> lock(lnmutex);
	nmap.erase(node);
}