#include "stdafx.h"
#include "node.h"
#include <cmath>
#include <algorithm>
#include <limits>

SearchNode::SearchNode(const Move& move)
	:move(move)
{
	state = State::NE;
	eval = 0;
	mass = 0;
}

void SearchNode::deleteTree() {
	std::vector<SearchNode*> nodes = children;
	while (!nodes.empty()) {
		SearchNode* node = nodes.back();
		nodes.pop_back();
		nodes.insert(nodes.end(), node->children.begin(), node->children.end());
		delete node;
	}
}

SearchNode* SearchNode::addChild(const Move& move) {
	SearchNode* child = new SearchNode(move);
	children.push_back(child);
	return child;
}

void SearchNode::setMateVariation(const double childmin) {
	if (childmin > 0) {
		eval = -childmin + mateOneScore;
		const double moves = (mateScore - childmin) / mateOneScore;
		mass = mateMass + moves;
	}
	else {
		eval = -childmin - mateOneScore;
		const double moves = (mateScore + childmin) / mateOneScore;
		mass = mateMass = moves;
	}
	state = State::MV;
}

void SearchNode::setMate() {
	eval = -mateScore;
	mass = mateMass;
	state = State::CM;
}

void SearchNode::setUchiFuMate() {
	eval = mateScore;
	mass = mateMass;
	state = State::CM;
}

void SearchNode::setDeclare() {
	eval = mateScore;
	mass = mateMass;
	state = State::CM;
}

void SearchNode::setRepetition(const double m) {
	eval = repetitionScore;
	mass = m;
	state = State::RP;
}

void SearchNode::setRepetitiveCheck(const double m) {
	eval = mateScore;
	mass = m;
	state = State::RC;
}