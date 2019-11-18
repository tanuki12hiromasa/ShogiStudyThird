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
	:move(move), expanded(false)
{
	state = State::N;
	eval = 0;
	mass = 0;
}

size_t SearchNode::deleteTree() {
	if (children.empty()) {
		return 0;
	}
	std::vector<SearchNode*> nodes = children;
	children.clear();
	size_t delnum = nodes.size();
	while (!nodes.empty()) {
		SearchNode* node = nodes.back();
		nodes.pop_back();
		delnum += node->children.size();
		nodes.insert(nodes.end(), node->children.begin(), node->children.end());
		delete node;
	}
	state = State::N;
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
		mass = mateMass + moves;
	}
}

void SearchNode::setMate() {
	auto const from = move.from();
	if (from == koma::Position::m_sFu || from == koma::Position::m_gFu) {
		eval = mateScore;
	}
	else {
		eval = -mateScore;
	}
	mass = mateMass;
	state = State::T;
}

void SearchNode::setDeclare() {
	eval = mateScore;
	mass = mateMass;
	state = State::T;
}

void SearchNode::setRepetition(const double m) {
	eval = repetitionScore;
	mass = m;
	state = State::T;
}

void SearchNode::setRepetitiveCheck(const double m) {
	eval = mateScore;
	mass = m;
	state = State::T;
}

double SearchNode::getT_c() const {
	return 60.0 * (mass / 2.0 + 0.5);
}
