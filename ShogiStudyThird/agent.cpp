#include "stdafx.h"
#include "agent.h"
#include <algorithm>

SearchAgent::SearchAgent(SearchTree& tree, unsigned threadid, int seed)
	:tree(tree), ID(threadid),engine(seed)
{
	alive = true;
}

SearchAgent::SearchAgent(SearchAgent&& agent) noexcept
	: tree(agent.tree), ID(agent.ID),
	engine(std::move(agent.engine))
{
	alive = agent.alive.load();
}

void SearchAgent::loop() {
	size_t newnodecount = 0;
	while (alive) {
		SearchNode* root = tree.getRoot(ID, newnodecount);
		if (root != nullptr) {
			newnodecount = simulate(root);
		}
		else {
			newnodecount = 0;
			std::this_thread::sleep_for(std::chrono::microseconds(200));
		}
	}
}

size_t SearchAgent::simulate(SearchNode* const root) {
	using ChildN = std::pair<double, SearchNode*>;
	const double T_c = tree.getTchoice();
	const double T_e = tree.getTeval();
	const double T_d = tree.getTdepth();
	const double MateScoreBound = SearchNode::getMateScoreBound();
	size_t newnodecount = 0;
	SearchNode* node = root;
	SearchPlayer player(tree.getRootPlayer());
	std::vector<SearchNode*> history = { node };
	//選択
	while (!node->isLeaf()) {
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
		history.push_back(node);
	}
	//末端ノードが他スレッドで展開中になっていないかチェック
	if (!tree.resisterLeafNode(node)) {
		return 0;
	}
	//展開・評価
	{
		std::vector<SearchNode*> gennodes;
		if (player.kyokumen.isDeclarable()) {
			node->setDeclare();
			goto backup;
		}
		if (false/*千日手である*/) {
			if (false/*連続王手である*/) {
				node->setRepetitiveCheck(1);
			}
			else {
				node->setRepetition(1);
			}
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
		newnodecount += gennodes.size();
		node->state = SearchNode::State::EQ;
		Evaluator::evaluate(gennodes, player);
		std::sort(node->children.begin(), node->children.end(), [](SearchNode* a, SearchNode* b)->int {return a->eval < b->eval; });
	
		//今展開したノードから静止探索
		{
			const double qsmassmax = tree.getMQS();
			const double T_cq = tree.getTcQ();
			while (node->isLeaf() && node->mass < qsmassmax) {
				SearchNode* qnode = node;
				SearchPlayer qplayer = player;
				std::vector<SearchNode*> qhistory = { qnode };
				//選択
				while (!qnode->isNotExpanded()) {
					double emin = std::numeric_limits<double>::max();
					std::vector<ChildN> evals;
					for (auto child : qnode->children) {
						if (!qnode->isQSTerminal()) {
							const double eval = child->getEvaluation();
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
					newnodecount += gennodes.size();
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
							qnode->state = SearchNode::State::LT;
						}
						goto qbackup;
					}
					node->state = SearchNode::State::LE;
					//評価,展開ノードの評価値バックアップ
					Evaluator::evaluate(gennodes, qplayer);
					{
						double emin = std::numeric_limits<double>::max();
						std::vector<double> evals;
						for (const auto& child : qnode->children) {
							const double eval = child->getEvaluation();
							evals.push_back(eval);
							if (eval < emin) {
								emin = child->getEvaluation();
							}
						}
						double Z_e = 0;
						double Z_d = 0;
						for (const auto& eval : evals) {
							Z_e += std::exp(-(eval - emin) / T_e);
							Z_d += std::exp(-(eval - emin) / T_d);
						}
						double E = 0;
						double M = 0;
						auto cit = qnode->children.begin();
						for (const auto& eval : evals) {
							double mass = (*cit)->mass;
							E -= eval * std::exp(-(eval - emin) / T_e) / Z_e;
							M += mass * std::exp(-(eval - emin) / T_d) / Z_d;
							cit++;
						}
						qnode->setEvaluation(E);
						qnode->setMass(M);
					}
				}
				//バックアップ
			qbackup:
				for (int i = qhistory.size() - 1; i >=0 ;i--) {
					qnode = qhistory[i];
				//もし途中でLEノードが詰みになってしまったら、そのノードをフル展開する
					double emin = std::numeric_limits<double>::max();
					std::vector<double> evals;
					bool allterminal = true;
					for (const auto child : qnode->children) {
						const double eval = child->eval;
						evals.push_back(eval);
						if (eval < emin) {
							emin = eval;
						}
						if (!qnode->isQSTerminal()) {
							allterminal = false;
						}
					}
					if (std::abs(emin) > MateScoreBound) {
						if (qnode->isLimitedExpanded()) {
							if (emin < 0) {
								//勝ちの詰みなので詰み確定 生成していない合法手は残っている可能性はあるが不要
								qnode->setMateVariation(emin);
							}
							else {
								//負けの詰みなので残りのノードの評価値も参照する
								SearchPlayer tplayer = player;
								for (int j = 1; j <= i; j++) {
									tplayer.proceed(qhistory[j]->move);
								}
								//tplayerをqhistoryを使って現在局面までもってくる

								std::vector<SearchNode*> gennodes;
								gennodes = MoveGenerator::genNocapMove(qnode, tplayer.kyokumen);
								newnodecount += gennodes.size();
								if (!gennodes.empty()) {
									Evaluator::evaluate(gennodes, tplayer);
									double emin = std::numeric_limits<double>::max();
									std::vector<double> evals;
									for (const auto& child : qnode->children) {
										const double eval = child->getEvaluation();
										evals.push_back(eval);
										if (eval < emin) {
											emin = child->getEvaluation();
										}
									}
									double Z_e = 0;
									double Z_d = 0;
									for (const auto& eval : evals) {
										Z_e += std::exp(-(eval - emin) / T_e);
										Z_d += std::exp(-(eval - emin) / T_d);
									}
									double E = 0;
									double M = 0;
									auto cit = qnode->children.begin();
									for (const auto& eval : evals) {
										double mass = (*cit)->mass;
										E -= eval * std::exp(-(eval - emin) / T_e) / Z_e;
										M += mass * std::exp(-(eval - emin) / T_d) / Z_d;
										cit++;
									}
									qnode->setEvaluation(E);
									qnode->setMass(M);
									qnode->state = SearchNode::State::EQ;
								}
								else {
									qnode->setMateVariation(emin);
								}
							}
						}
						else {
							qnode->setMateVariation(emin);
						}
					}
					else {
						double Z_e = 0;
						double Z_d = 0;
						for (const auto eval : evals) {
							Z_e += std::exp(-(eval - emin) / T_e);
							Z_d += std::exp(-(eval - emin) / T_d);
						}
						double E = 0;
						double M = 0;
						auto cit = qnode->children.begin();
						for (const auto& eval : evals) {
							double mass = (*cit)->mass;
							E -= eval * std::exp(-(eval - emin) / T_e) / Z_e;
							M += mass * std::exp(-(eval - emin) / T_d) / Z_d;
							cit++;
						}
						qnode->setEvaluation(E);
						qnode->setMass(M);
						if (allterminal) {
							if (qnode->state == SearchNode::State::LE) {
								qnode->state = SearchNode::State::LT;
							}
							else if (qnode->state == SearchNode::State::EQ) {
								qnode->state = SearchNode::State::ET;
							}
						}
					}

				}

			}//静止探索1ループここまで

		}//静止探索ここまで

		tree.excludeLeafNode(node);
	}//展開評価ここまで

	//バックアップ
	backup:
	{
	for (int i = history.size() - 1; i >= 0; i--) {
			node = history[i];
			double emin = std::numeric_limits<double>::max();
			std::vector<double> evals;
			for (const auto& child : node->children) {
				const double eval = child->eval;
				evals.push_back(eval);
				if (eval < emin) {
					emin = eval;
				}
			}
			if (std::abs(emin) > MateScoreBound) {
				node->setMateVariation(emin);
			}
			else {
				double Z_e = 0;
				double Z_d = 0;
				for (const auto& eval : evals) {
					Z_e += std::exp(-(eval - emin) / T_e);
					Z_d += std::exp(-(eval - emin) / T_d);
				}
				double E = 0;
				double M = 0;
				auto cit = node->children.begin();
				for (const auto& eval : evals) {
					double mass = (*cit)->mass;
					E -= eval * std::exp(-(eval - emin) / T_e) / Z_e;
					M += mass * std::exp(-(eval - emin) / T_d) / Z_d;
					cit++;
				}
				node->setEvaluation(E);
				node->setMass(M);
			}
		}
	}

	return newnodecount;
}
