#include "stdafx.h"
#include "agent.h"
#include "move_gen.h"
#include <algorithm>

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
	const double T_e = tree.getTeval();
	SearchNode* node = root = tree.getRoot();
	SearchPlayer player(tree.getRootPlayer());
	//下り
	yarinaoshi:
	while (!node->isLeaf()) {
		//ノード選択
		double CE = std::numeric_limits<double>::max();
		std::vector<double> evals;
		for (const auto& child : node->children) {
			double eval = child->getEvaluation();
			evals.push_back(eval);
			if (eval < CE)
				CE = eval;
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
		player.proceed(node->move);
	}
	{
		std::mutex& mtx = tree.getMutex(node);
		std::lock_guard<std::mutex> lock(mtx);
		if (!node->isLeaf()) {
			tree.restoreMutex(node);
			goto yarinaoshi;
		}
		//展開・評価
		{
			std::vector<SearchNode*> gennodes;
			if (node->isNotExpanded()) gennodes = MoveGenerator::genMove(node, player.kyokumen);
			else gennodes = MoveGenerator::genNocapMove(node, player.kyokumen);
			Evaluator::evaluate(gennodes, player);
			node->state = SearchNode::State::LE;
			std::sort(node->children.begin(), node->children.end(), [](SearchNode* a, SearchNode* b)->int {return a->getEvaluation() < b->getEvaluation(); });
		}
		//今展開したノードから静止探索
		{
			const double qsmassmax = tree.getMQS();
			const double T_cq = tree.getTcQ();
			while (!node->isQSTerminal() && node->mass < qsmassmax) {
				SearchNode* qnode = node;
				while (!qnode->isNotExpanded()) {
					double emin = std::numeric_limits<double>::max();
					std::vector<ChildN> evals;
					for (auto child : qnode->children) {
						if (!qnode->isQSTerminal) {
							const double eval = child->getEvaluation();
							evals.emplace_back(std::make_pair(eval, child));
							if (eval < emin) {
								emin = eval;
							}
						}
					}
					double Z = 0;
					
				}
				//展開
				
				//評価
				
				//バックアップ
				//もし途中でLEノードが詰みになってしまったら、そのノードをフル展開する

			}
		}
		//バックアップ

		tree.restoreMutex(node);
	}



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