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
		double evalmin = std::numeric_limits<double>::max();
		std::vector<ChildN> evals;
		{
			s_lock(node->_mutex);
			for (const auto& child : node->children) {
				double eval = child->getEvaluation();
				evals.push_back(ChildN(eval, child));
				if (eval < evalmin)evalmin = eval;
			}
		}
		double Z = 0;
		for (const auto cn : evals) {
			Z += std::exp(-(cn.first - evalmin) / T_c);
		}



	}
	//展開

	//
	//
	//
	//上り
}