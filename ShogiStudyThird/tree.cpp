#include "stdafx.h"
#include "tree.h"


SearchTree::SearchTree() {

}

void SearchTree::clear() {

}

void SearchTree::proceed(SearchNode* node) {

}

void SearchTree::setTchoice(const std::vector<double>& T) {
	const size_t size = T.size();
	for (size_t i = 0; i < T_choice.size(); i++) {
		T_choice[i] = T[i % size];
	}
}

double SearchTree::getTchoice() {
	const unsigned long long count = T_c_count++ % 64;
	return T_choice[count];
}

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