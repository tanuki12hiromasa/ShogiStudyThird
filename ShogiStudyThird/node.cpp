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
double SearchNode::Tc_const = 60;
double SearchNode::Tc_mp = 30;
double SearchNode::Tc_mc = 20;
bool SearchNode::Tc_mc_expectable_variance = false;
double SearchNode::T_eval = 40;
double SearchNode::T_depth = 90;
double SearchNode::MassMax_QS = 8;
int SearchNode::Ec_FunctionCode = 0;
double SearchNode::Ec_c = 200.0;

SearchNode::SearchNode(const Move& move)
	:move(move), expanded(false)
{
	status = State::N;
	eval = 0;
	mass = 0;
	visit_count = 0;
}

size_t SearchNode::deleteTree() {
	if (children.empty()) {
		return 0;
	}
	std::vector<SearchNode*> nodes = children;
	children.clear();
	status = State::N;
	expanded = false;
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
	status = State::T;
}

void SearchNode::setDeclare() {
	eval = mateScore;
	mass = mateMass;
	status = State::T;
}

void SearchNode::setRepetition(const bool teban) {
	deleteTree();
	eval = teban ? repetitionScore : (-repetitionScore);
	mass = mateMass;
	status = State::T;
}

void SearchNode::setRepetitiveCheck() {
	deleteTree();
	eval = mateScore;
	mass = mateMass;
	status = State::T;
}

double SearchNode::getT_c() const {
	if (Tc_mp != 0) {
		if (Tc_mc != 0) {
			return Tc_const + Tc_mp * (mass - 1) * Tc_mc * getTcMcVariance();
		}
		else {
			return Tc_const + Tc_mp * (mass - 1);
		}
	}
	else {
		if (Tc_mc != 0) {
			return Tc_const + Tc_mc * getTcMcVariance();
		}
		else {
			return Tc_const;
		}
	}
}

double SearchNode::getTcMcVariance()const {
	if (Tc_mc_expectable_variance) {
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
	else {
		std::vector<std::pair<double,double>> ems;
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
}

double SearchNode::getE_c(const size_t& visitnum_p, const double& mass_p)const {
	switch (Ec_FunctionCode)
	{
	case 0:
		return eval;
	case 1:
		return eval - Ec_c * (std::log((double)visitnum_p) / (visit_count+1));
	case 2:
		return eval + Ec_c * origin_eval * (double)visitnum_p / (visit_count * visit_count + 1);
	case 3:
		return eval - Ec_c * mass_p / std::exp(mass);
	case 4:
	{
		const double m = mass.load();
		return eval - Ec_c * mass_p / (1 + m * m * m);
	}
	case 5:
	{
		const double m = mass.load();
		return eval + Ec_c * origin_eval * mass_p / (1 + m * m * m);
	}
	case 6:
	{
		const double m = mass.load();
		return eval * (1 + Ec_c * mass_p / (1 + m * m * m));
	}
	case 7:
		return eval + Ec_c * origin_eval * std::exp(mass_p / 2 - mass);
	case 8:
		return eval - Ec_c * std::exp(mass_p / 2 - mass);
	case 9:
		return eval + Ec_c * origin_eval;
	case 10:
		return eval * (1 + Ec_c * std::exp(mass_p / 2 - mass));
	case 11:
		return eval * (1 + Ec_c * (double)visitnum_p / (visit_count * visit_count + 1));
	case 12:
	{
		const double e = eval.load();
		return e + ((e > 0) ? -mass  * Ec_c : mass * Ec_c);
	}
	case 13:
		return eval * (1 - Ec_c * mass);
	case 14:
	{
		const double m = mass.load();
		return eval - Ec_c * mass_p / (1 + m * m);
	}
	case 15:
		return eval + Ec_c * mass;
	case 16: {
		double p = Ec_c / (mass + 1);
		return eval * (1.0 - p) + origin_eval * p;
	}
	case 17: {
		double p = Ec_c / std::sqrt(mass + 1);
		return eval * (1.0 - p) + origin_eval * p;
	}
	case 18: {
		const double x = mass.load();
		double p = (x >= 1) ? (Ec_c / x) : 1;
		return eval * (1.0 - p) + origin_eval * p;
	}
	default:
		return eval;
	}
}