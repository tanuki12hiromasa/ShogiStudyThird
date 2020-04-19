#include "stdafx.h"
#include "node.h"
#include <cmath>
#include <algorithm>
#include <limits>

double SearchNode::mateMass = 1;
double SearchNode::mateScore = 34000.0;//詰ませた側(勝った側)のscore
double SearchNode::mateScoreBound = 30000.0;
double SearchNode::mateOneScore = 20.0;
double SearchNode::repetitionScore = -100;//先手側のscore（千日手のscoreは手番に依存する）
double SearchNode::Ts_c = 1.0;
int SearchNode::Ts_FunctionCode = 0;
double SearchNode::T_eval = 40;
double SearchNode::T_depth = 90;
int SearchNode::QS_depth = 0;
int SearchNode::Es_FunctionCode = 0;
double SearchNode::Es_c = 1.0;
int SearchNode::PV_FuncCode = 0;
double SearchNode::PV_c = 0;

SearchNode::SearchNode(const Move& move)
	:move(move)
{
	status = State::N;
	eval = 0;
	mass = 0;
}

size_t SearchNode::deleteTree() {
	if (children.empty()) {
		return 0;
	}
	std::vector<SearchNode*> nodes = children;
	children.clear();
	status = State::Terminal;
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

SearchNode* SearchNode::addCopyChild(const SearchNode* const origin) {
	SearchNode* child = new SearchNode(origin->move);
	child->eval = origin->eval.load();
	child->origin_eval = origin->origin_eval;
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

double SearchNode::getTs(const double baseT) const {
	switch (Ts_FunctionCode)
	{
	case 0:
	default:
		return baseT;
	case 1:
		return baseT * std::pow(Ts_c, mass.load());
	}
}

SearchNode* SearchNode::restoreNode(const Move& move, int st, double eval, double mass)
{
	SearchNode* child = new SearchNode(move);
	child->eval = eval;
	child->mass = mass;
	child->status = (SearchNode::State)st;

	return child;
}

//子供を評価値順に並べ替えていく。読み込み前後で木が同一であることを確認するために使用
void SearchNode::sortChildren(SearchNode* node) {
	//sortは静止探索後の方が評価値順の並びが維持されやすい　親スタートの静止探索ならその前後共にsortしてもいいかもしれない
	std::sort(node->children.begin(), node->children.end(), [](SearchNode* a, SearchNode* b)->int {return a->eval < b->eval; });
	for (auto c : node->children) {
		sortChildren(c);
	}
}

double SearchNode::getTcMcVariance()const {
	std::vector<double> cmasses;
	double mean = 0;
	for (const auto& child : children) {
		const double m = child->mass;
		cmasses.push_back(m);
		mean += m;
	}
	mean /= cmasses.size();
	double variance = 0;
	for (const auto& m : cmasses) {
		variance += (m - mean) * (m - mean);
	}
	return std::sqrt(variance / cmasses.size());
}
double SearchNode::getTcMcVarianceExpection()const {
	std::vector<std::pair<double, double>> ems;
	double min = std::numeric_limits<double>::max();
	for (const auto& child : children) {
		const double e = child->eval;
		ems.push_back(std::make_pair(e, child->mass.load()));
		if (e < min) {
			min = e;
		}
	}
	double Z = 0;
	for (auto& e : ems) {
		const double exp = std::exp(-(e.first - min) / T_depth);
		Z += exp;
		e.first = exp;
	}
	const double mean = mass - 1;
	double variance = 0;
	for (const auto& e : ems) {
		variance += (e.second - mean) * (e.second - mean) * e.first;
	}
	return std::sqrt(variance / Z);
}


double SearchNode::getEs()const {
	switch (Es_FunctionCode)
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

SearchNode* SearchNode::getBestChild()const {
	SearchNode* best = nullptr;
	if (children.empty())return nullptr;
	switch (PV_FuncCode) {
		default:
		case 0: {
			double min = std::numeric_limits<double>::max();
			for (const auto child : children) {
				const double e = child->eval - child->mass * PV_c;
				if (e < min) {
					best = child;
					min = e;
				}
			}
			return best;
		}
		case 1: {
			using ddn = std::tuple<double, double, SearchNode*>;
			double emin = std::numeric_limits<double>::max();
			std::vector<ddn> emnvec;
			for (const auto& child : children) {
				const double e = child->eval;
				const double m = child->mass;
				emnvec.push_back(std::make_tuple(e, m, child));
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
			for (const auto child : children) {
				const double x = child->mass.load();
				double p = PV_c * ((x >= 1) ? (1 / x) : 1);
				const double e = child->eval * (1.0 - p) + origin_eval * p;
				if (e < min) {
					best = child;
					min = e;
				}
			}
			return best;
		}
		case 3: {
			const double dbound = (mass - 1) * PV_c;
			SearchNode* best = nullptr;
			double min = std::numeric_limits<double>::max();
			for (const auto child : children) {
				const double ce = child->eval;
				if (child->mass >= dbound && ce < min) {
					min = ce;
					best = child;
				}
			}
			if (best != nullptr) {
				return best;
			}
			else {
				for (const auto child : children) {
					const double ce = child->eval;
					if (ce < min) {
						min = ce;
						best = child;
					}
				}
				return best;
			}
		}
	}
}