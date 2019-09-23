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
	searchstart:
	const double T_c = tree.getTchoice();
	const double T_e = tree.getTeval();
	const double T_d = tree.getTdepth();
	SearchNode* node = root = tree.getRoot();
	SearchPlayer player(tree.getRootPlayer());
	std::vector<SearchNode*> history = tree.getHistory();
	//選択
	while (!node->isLeaf()) {
		double CE = std::numeric_limits<double>::max();
		std::vector<double> evals;
		for (const auto& child : node->children) {
			double eval = child->getChoiceEvaluation();
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
		history.push_back(node);
	}
	//末端ノードが他スレッドで展開中になっていないかチェック
	if (!tree.resisterLeafNode(node)) {
		goto searchstart;
	}
	//展開・評価
	{
		std::vector<SearchNode*> gennodes;
		if (player.kyokumen.isDeclarable()) {
			node->setDeclare();
			goto backup;
		}
		switch (node->state)
		{
		case SearchNode::State::NE:
			gennodes = MoveGenerator::genMove(node, player.kyokumen);
			break;
		case SearchNode::State::LE:
		case SearchNode::State::LT:
			gennodes = MoveGenerator::genNocapMove(node, player.kyokumen);
			break;
		//case EQ は全て展開済みなので手生成不要
		}
		if (node->children.empty()) {
			auto from = node->move.from();
			if (from == koma::Position::m_sFu || from == koma::Position::m_gFu) {
				node->setUchiFuMate();
			}
			else {
				node->setMate();
			}
			goto backup;
		}
		node->state = SearchNode::State::EQ;
		Evaluator::evaluate(gennodes, player);
		std::sort(node->children.begin(), node->children.end(), [](SearchNode* a, SearchNode* b)->int {return a->eval < b->eval; });
	
		//今展開したノードから静止探索
		{
			const double qsmassmax = tree.getMQS();
			const double T_cq = tree.getTcQ();
			while (!node->isQSTerminal() && node->mass < qsmassmax) {
				SearchNode* qnode = node;
				SearchPlayer qplayer = player;
				std::vector<SearchNode*> qhistory = history;
				//選択
				while (!qnode->isNotExpanded()) {
					double emin = std::numeric_limits<double>::max();
					std::vector<ChildN> evals;
					for (auto child : qnode->children) {
						if (!qnode->isQSTerminal) {
							const double eval = child->getChoiceEvaluation();
							evals.emplace_back(std::make_pair(eval, child));
							if (eval < emin) {
								emin = eval;
							}
						}
					}
					double Z = 0;
					for (const auto& dn : evals) {
						Z += std::exp(-(dn.first - emin) / T_cq);
					}
					double pip = Z * random(engine);
					qnode = evals.front().second;
					for (const auto& dn : evals) {
						pip -= std::exp(-(dn.first - emin) / T_cq);
						if (pip <= 0) {
							qnode = dn.second;
							break;
						}
					}
					qplayer.proceed(qnode->move);
					qhistory.push_back(qnode);
				}
				//展開
				{
					if (qplayer.kyokumen.isDeclarable()) {
						qnode->setDeclare();
						goto qbackup;
					}
					std::vector<SearchNode*> gennodes;
					gennodes = MoveGenerator::genCapMove(qnode, qplayer.kyokumen);
					if (gennodes.empty()) {
						if (node->move.isOute()) {
							const koma::Position from = node->move.from();
							if (from == koma::Position::m_sFu || from == koma::Position::m_gFu) {
								node->setUchiFuMate();
							}
							else {
								node->setMate();
							}
						}
						else {
							node->state = SearchNode::State::LT;
						}
						goto qbackup;
					}
					node->state = SearchNode::State::LE;
					//評価,展開ノードの評価値バックアップ
					Evaluator::evaluate(gennodes, qplayer);
					{
						double CE = std::numeric_limits<double>::max();
						std::vector<double> evals;
						for (const auto& child : qnode->children) {
							evals.push_back(child->eval);
							if (child->getChoiceEvaluation() < CE) {
								CE = child->getChoiceEvaluation();
							}
						}
						double eZ = 0;
						double dZ = 0;
						for (const auto& eval : evals) {
							eZ += std::exp(-(eval - CE) / T_e);
							dZ += std::exp(-(eval - CE) / T_d);
						}
						double E = 0;
						double M = 0;
						auto cit = qnode->children.begin();
						for (const auto& eval : evals) {
							double mass = (*cit)->mass;
							E -= eval * std::exp(-(eval - CE) / T_e) / eZ;
							M += mass * std::exp(-(eval - CE) / T_d) / dZ;
							cit++;
						}
					}
				}
				//バックアップ
			qbackup:
				auto qnit = qhistory.rbegin();
				do{
					qnode = *(++qnit);
				//もし途中でLEノードが詰みになってしまったら、そのノードをフル展開する
					double emin = std::numeric_limits<double>::max();
					std::vector<double> evals;
					bool allterminal = true;

				} while (qnode != node);

			}//静止探索1ループここまで

		}//静止探索ここまで

		tree.excludeLeafNode(node);
	}//展開評価ここまで


	//バックアップ
	backup:
	{
	/*千日手ノードの評価値をバックアップに含めて考慮するため、
	evalsにはnode->evalを、CEにはnode->getChoiceEvaluation()を格納する
	(千日手評価値は最善であるときのみ反映されるため)*/
	
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
}
