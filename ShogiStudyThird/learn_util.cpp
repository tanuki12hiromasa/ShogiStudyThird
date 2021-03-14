#include "learn_util.h"
#include <iostream>

double LearnUtil::pTb = 0.1;//評価関数をシグモイド関数に写像した場合の方策の温度は、(元の温度)*(写像先の標準的な評価値差)/(元の標準的な評価値差)で求められる。


SearchNode* LearnUtil::choicePolicyRandomChild(const SearchNode* const root, const double T, double pip) {
	using dn = std::pair<double, SearchNode*>;
	double CE = std::numeric_limits<double>::max();
	if (root->isLeaf()) return nullptr;
	std::vector<dn> evals; evals.reserve(root->children.size());
	for (auto& child : root->children) {
		const double eval = child.eval;
		evals.push_back(std::make_pair(eval, &child));
		if (eval < CE) {
			CE = eval;
		}
	}
	if (evals.empty()) {
		return nullptr;
	}
	double Z = 0;
	for (const auto& eval : evals) {
		Z += std::exp(-(eval.first - CE) / T);
	}
	pip *= Z;
	for (const auto& eval : evals) {
		pip -= std::exp(-(eval.first - CE) / T);
		if (pip <= 0) {
			return eval.second;
		}
	}
	return evals.front().second;
}

SearchNode* LearnUtil::choiceRandomChild(const SearchNode* const root, const double pip) {
	const int index = std::clamp(static_cast<int>(root->children.size() * pip), 0, (int)root->children.size() - 1);
	return root->children.begin() + index;
}

SearchNode* LearnUtil::choiceBestChild(const SearchNode* const root) {
	double min = std::numeric_limits<double>::max();
	SearchNode* best = nullptr;
	for (auto& child : root->children) {
		double eval = child.getEs();
		if (eval < min) {
			min = eval;
			best = &child;
		}
	}
	return best;
}

SearchNode* LearnUtil::getPrincipalLeaf(const SearchNode* const root) {
	const auto child = choiceBestChild(root);
	if (child == nullptr || child->isTerminal() || child->isLeaf())
		return child;
	else
		return getPrincipalLeaf(child);
}


double LearnUtil::EvalToProb(const double eval) {
	return 1.0 / (1.0 + std::exp(-eval / probT));
}

double LearnUtil::BackProb(const SearchNode& parent, const SearchNode& child, const double T) {
	double Z = 0;
	for (const auto& c : parent.children) {
		Z += std::exp(-(c.eval + parent.eval) / T);
	}
	return std::exp(-(child.eval + parent.eval) / T) / Z;
}

std::string LearnUtil::getDateString() {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string s(30, '\0');
	std::tm ima;
	localtime_s(&ima, &now);
	std::strftime(&s[0], s.size(), "%Y%m%d-%H%M", &ima);
	s.resize(s.find('\0'));
	return s;
}