#include "stdafx.h"
#include "node.h"
#include <cmath>
#include <algorithm>
#include <limits>

double SearchNode::mateMass = 9999999;
double SearchNode::mateScore = 34000.0;//詰ませた側(勝った側)のscore
double SearchNode::mateScoreBound = 30000.0;
double SearchNode::mateOneScore = 20.0;
double SearchNode::repetitionScore = -100;//先手側のscore（千日手のscoreは手番に依存する）
int SearchNode::QS_depth = 8;
int SearchNode::Es_FunctionCode = 0;
double SearchNode::Es_c = 1.0;
int SearchNode::PV_FuncCode = 0;
double SearchNode::PV_c = 0;
std::atomic_int64_t SearchNode::nodecount{ 0 };

SearchNode::Children::~Children() {
	if (list) {
		delete[] list;
		nodecount -= count;
	}
}

void SearchNode::Children::sporn(const std::vector<Move>& moves) {
	count = moves.size();
	list = new SearchNode[count];
	for (int i = 0; i < count; i++) {
		list[i].move = moves[i];
	}
	nodecount += count;
}

void SearchNode::Children::clear() {
	if (list) {
		delete[] list;
		list = nullptr;
		nodecount -= count;
	}
	count = 0;
}


void SearchNode::Children::sort(SearchNode* list, int l, int h) {
	if (l >= h)return;
	auto i = l, j = h;
	auto x = list[(i + j) / 2].eval.load();
	while (true) {
		while (list[i].eval.load() < x)i++;
		while (x < list[j].eval.load())j--;
		if (i >= j)break;
		list[i].swap(list[j]);
		i++; j--;
	}
	sort(list, l, i - 1);
	sort(list, j + 1, h);
}

void SearchNode::Children::sort() {
	if (empty())return;
	//評価値順にソートする(クイックソート)
	sort(list, 0, count - 1);
}

void SearchNode::Children::swap(SearchNode::Children& children) {
	if (list && children.list) {
		const auto temp_count = count;
		count = children.count; children.count = temp_count;
		const auto temp_list = list;
		list = children.list; children.list = temp_list;
	}
}

SearchNode::Children* SearchNode::Children::purge() {
	Children* c = new Children();
	c->list = list;
	c->count = count;
	list = nullptr;
	count = 0;
	return c;
}

SearchNode::SearchNode():origin_eval(0) {
	status = State::N;
	eval = 0;
	mass = 0;
}

SearchNode::SearchNode(const Move& move)
	:move(move)
{
	status = State::N;
	eval = 0;
	mass = 0;
}

void SearchNode::swap(SearchNode& node) {
	children.swap(node.children);
	const auto temp_move = move;
	move = node.move; node.move = temp_move;
	if (status.load() != node.status.load()) {
		const auto temp_status = status.load();
		status = node.status.load(); node.status = temp_status;
	}
	std::swap(origin_eval, node.origin_eval);
	const auto temp_eval = eval.load();
	eval = node.eval.load(); node.eval = temp_eval;
	if (mass.load() != node.mass.load()) {
		const auto temp_mass = mass.load();
		mass = node.mass.load(); node.mass = temp_mass;
	}
}

SearchNode::Children* SearchNode::purge() {
	status = State::NotExpanded;
	return children.purge();
}

void SearchNode::addChildren(const std::vector<Move>& moves) {
	children.sporn(moves);
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
	origin_eval = eval;
}

void SearchNode::setMate() {
	auto const from = move.from();
	if (from == koma::Position::m_sFu || from == koma::Position::m_gFu) {
		eval = mateScore;
		origin_eval = mateScore;
	}
	else {
		eval = -mateScore;
		origin_eval = -mateScore;
	}
	mass = mateMass;
	status = State::T;
}

void SearchNode::setDeclare() {
	eval = mateScore;
	origin_eval = mateScore;
	mass = mateMass;
	status = State::T;
}

void SearchNode::setRepetition(const bool teban) {
	//deleteTree();
	eval = teban ? repetitionScore : (-repetitionScore);
	origin_eval = eval.load();
	mass = mateMass;
	status = State::T;
}

void SearchNode::setRepetitiveCheck() {
	//deleteTree();
	eval = mateScore;
	origin_eval = mateScore;
	mass = mateMass;
	status = State::T;
}

double SearchNode::getEs(int funccode)const {
	switch (funccode)
	{
	case 0:
	default:
		return eval;
	case 9:
		return eval + Es_c * origin_eval;
	case 10:
	case 11:
	case 12:
	{
		const double e = eval.load();
		return e + ((e > 0) ? -mass  * Es_c : mass * Es_c);
	}
	case 13:
		return eval * (1 - Es_c * mass);
	case 14:
	case 15:
		return eval + Es_c * mass;
	case 16: {
		double p = Es_c / (mass + 1);
		return eval * (1.0 - p) + origin_eval * p;
	}
	case 17: {
		double p = Es_c / std::sqrt(mass + 1);
		return eval * (1.0 - p) + origin_eval * p;
	}
	case 18: {
		const double x = mass.load();
		double p = Es_c * ((x >= 1) ? (1 / x) : 1);
		return eval * (1.0 - p) + origin_eval * p;
	}
	case 19:
		return eval * (1 - Es_c) + origin_eval * Es_c;
	case 20: {
		const double x = mass.load();
		double p = Es_c * ((x >= 1) ? (1 / (x*x)) : 1);
		return eval * (1.0 - p) + origin_eval * p;
	}
	}
}

SearchNode* SearchNode::getBestChild(int funccode)const {
	SearchNode* best = nullptr;
	if (children.empty())return nullptr;
	switch (funccode) {
		default:
		case 0: {
			double min = std::numeric_limits<double>::max();
			for (auto& child : children) {
				const double e = child.eval - child.mass * PV_c;
				if (e < min) {
					best = &child;
					min = e;
				}
			}
			return best;
		}
		case 1: {
			using ddn = std::tuple<double, double, SearchNode*>;
			double emin = std::numeric_limits<double>::max();
			std::vector<ddn> emnvec;
			for (auto& child : children) {
				const double e = child.eval;
				const double m = child.mass;
				emnvec.push_back(std::make_tuple(e, m, &child));
				if (e < emin) {
					emin = e;
				}
			}
			double bestMass = -1;
			for (const auto& emn : emnvec) {
				const double score = (std::get<1>(emn) + 1) * std::exp(-(std::get<0>(emn) - emin) / PV_c);
				if (score > bestMass) {
					bestMass = score;
					best = std::get<2>(emn);
				}
			}
			return best;
		}
		case 2: {
			double min = std::numeric_limits<double>::max();
			for (auto& child : children) {
				const double x = child.mass.load();
				double p = PV_c * ((x >= 1) ? (1 / x) : 1);
				const double e = child.eval * (1.0 - p) + origin_eval * p;
				if (e < min) {
					best = &child;
					min = e;
				}
			}
			return best;
		}
		case 3: {
			const double dbound = (mass - 1) * PV_c;
			SearchNode* best = nullptr;
			double min = std::numeric_limits<double>::max();
			for (auto& child : children) {
				const double ce = child.eval;
				if ((ce <= -mateScoreBound || min >= mateScoreBound || child.mass >= dbound) && ce < min) {
					min = ce;
					best = &child;
				}
			}
			if (best != nullptr) {
				return best;
			}
			else {
				for (auto& child : children) {
					const double ce = child.eval;
					if (ce < min) {
						min = ce;
						best = &child;
					}
				}
				return best;
			}
		}
	}
}

double SearchNode::getChildRate(SearchNode* const child, const double T)const {
	using dn = std::pair<double, SearchNode*>;
	double emin = std::numeric_limits<double>::max();
	std::vector<dn> nodes; nodes.reserve(children.size());
	for (auto& c : children) {
		const double e = c.eval;
		nodes.push_back(std::make_pair(e, &c));
		if (e < emin) {
			emin = e;
		}
	}
	double Z = 0;
	double childexp = 0;
	for (const auto node : nodes) {
		const double exp = std::exp(-(node.first - emin) / T);
		Z += exp;
		if (node.second == child) {
			childexp = exp;
		}
	}
	return childexp / Z;
}

int SearchNode::getMateNum()const {
	const double e = eval.load();
	if (e >= mateScoreBound) {
		return (mateScore - e) / mateOneScore;
	}
	else if(e <= -mateScoreBound) {
		return (-mateScore - e) / mateOneScore;
	}
	else {
		return -std::numeric_limits<int>::min();
	}
}