#include "stdafx.h"
#include "agent.h"

SearchAgent::SearchAgent(SearchTree& tree, int seed)
	:tree(tree), engine(seed), root(nullptr)
{}

SearchAgent::SearchAgent(SearchAgent&& agent) noexcept
	: tree(agent.tree),
	enable(agent.enable.load()), alive(agent.alive.load()),
	engine(std::move(agent.engine))
{
	root.store(agent.root);
}


bool SearchAgent::immigrated() {
	return root.load() == tree.getRoot();
}


void SearchAgent::simulate() {
	using ChildN = std::pair<double, SearchNode*>;
	const double T_c = tree.getTchoice();
	SearchNode* node = root = tree.getRoot();
	Feature feature(tree.getRootFeature());
	Kyokumen kyokumen(tree.getRootKyokumen());
#ifdef EVAL_DIFF_ONLY
	Feature feature(tree.getRootFeature());
#endif
	//下り
	yarinaoushi:
	while (!node->isLeaf()) {
		//ノード選択
		double CE = std::numeric_limits<double>::max();
		std::vector<double> evals;
		{
			s_lock(node->_mutex);
			for (const auto& child : node->children) {
				double eval = child->getEvaluation();
				evals.push_back(eval);
				if (eval < CE)
					CE = eval;
			}
		}
		double Z = 0;
		for (const auto& eval : evals) {
			Z += std::exp(-(eval - CE) / T_c);
		}
		double pip = Z * random(engine);
		const auto& children = node->children;
		node = children.front();
		auto child = children.begin();
		for (const auto& eval : evals) {
			pip -= std::exp(-(eval - CE) / T_c);
			if (pip <= 0) {
				node = *child;
				break;
			}
			child++;
		}
		//局面を進める

	}
	//展開

	{
		/*
			while (!node->isLeaf()) {
		double CE = std::numeric_limits<double>::max();
		std::vector<ChildN> evals;
		{
			s_lock(node->_mutex);
			for (const auto& child : node->children) {
				double eval = child->getEvaluation();
				evals.push_back(ChildN(eval, child));
				if (eval < CE)
					CE = eval;
			}
		}
		double Z = 0;
		for (const auto& cn : evals) {
			Z += std::exp(-(cn.first - CE) / T_c);
		}
		double pip = Z * random(engine);
		node = evals.front().second;
		for (const auto& cn : evals) {
			pip -= std::exp(-(cn.first - CE) / T_c);
			if (pip <= 0) {
				node = cn.second;
				break;
			}
		}

	}
		*/
	}
	//
	//
	//
	//上り
}