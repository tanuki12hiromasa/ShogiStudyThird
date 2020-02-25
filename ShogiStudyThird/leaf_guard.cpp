#include "stdafx.h"
#include "leaf_guard.h"

std::mutex LeafGuard::mutex;

LeafGuard::LeafGuard(SearchNode* const node)
	:node(node)
{
	std::lock_guard<std::mutex> lock(mutex);
	if (node->status.load() == SearchNode::State::NotExpanded) {//先約がいない場合
		node->status = SearchNode::State::inExpanding;
		result = true;
	}
	else { //先約がいる場合
		result = false;
	}
}

LeafGuard::~LeafGuard() {
	std::lock_guard<std::mutex> lock(mutex);
	if (result && node->status.load() == SearchNode::State::inExpanding) {
		node->status = SearchNode::State::E;
	}
}