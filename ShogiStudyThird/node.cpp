#include "stdafx.h"
#include "node.h"
#include <cmath>
#include <algorithm>
#include <limits>

SearchNode::SearchNode(const Move& move)
	:move(move)
{
	isLeaf = true;
}

SearchNode* SearchNode::addChild(const Move& move) {
	SearchNode* child = new SearchNode(move);
	children.push_back(child);
	return child;
}

SearchNode* SearchNode::choiceNode(double pip, const double T)const {
	//std::log(std::numeric_limits<double>::max()) = 709.783 なので、expにこれ以上の値を入れるとオーバーフローする
	if (children.empty())return nullptr;
	double Z = 0;
	double CE = std::numeric_limits<double>::max();
	std::vector<double> E_children;
	for (const auto& child : children) {
		double M_c = child->mass;
		double E_c = child->eval;
		if (E_c < CE) CE = E_c;
		E_children.push_back(E_c);
	}
	for (auto E_c : E_children) {
		Z += std::exp(-(E_c - CE) / T);
	}
	pip *= Z;
	auto child = children.begin();
	for (auto E_c = E_children.begin(), end = E_children.end(); E_c != end; E_c++, child++) {
		pip -= std::exp(-(*E_c - CE) / T);
		if (pip <= 0)
			return *child;
	}
	return children.front();
}