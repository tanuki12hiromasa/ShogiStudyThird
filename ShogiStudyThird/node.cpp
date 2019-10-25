#include "stdafx.h"
#include "node.h"
#include <cmath>
#include <algorithm>
#include <limits>

double SearchNode::mateMass = 1;
double SearchNode::mateScore = 34000.0;
double SearchNode::mateScoreBound = 30000.0;
double SearchNode::mateOneScore = 20.0;
double SearchNode::repetitionScore = -100;

SearchNode::SearchNode(const Move& move)
	:move(move)
{
	state = State::NE;
	eval = 0;
	mass = 0;
}

size_t SearchNode::deleteTree() {
	std::vector<SearchNode*> nodes = children;
	size_t delnum = nodes.size();
	while (!nodes.empty()) {
		SearchNode* node = nodes.back();
		nodes.pop_back();
		delnum += node->children.size();
		nodes.insert(nodes.end(), node->children.begin(), node->children.end());
		delete node;
	}
	return delnum;
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