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
	using dn = std::pair<double, SearchNode*>;
	using dd = std::pair<double, double>;
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
		std::vector<dn> evals;
		for (const auto& child : node->children) {
			if (child->isSearchable()) {
				double eval = child->getEvaluation();
				evals.push_back(std::make_pair(eval,child));
				if (eval < CE) {
					CE = eval;
				}
			}
		}
		if (evals.empty()) {
			node->state = SearchNode::State::Terminal;
			return 0;
		}
		double Z = 0;
		for (const auto& eval : evals) {
			Z += std::exp(-(eval.first - CE) / T_c);
		}
		double pip = Z * random(engine);
		node = evals.front().second;
		for (const auto& eval : evals) {
			pip -= std::exp(-(eval.first - CE) / T_c);
			if (pip <= 0) {
				node = eval.second;
				break;
			}
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
		case SearchNode::State::N:
			gennodes = MoveGenerator::genMove(node, player.kyokumen);
			node->state = SearchNode::State::QE;
			break;
		case SearchNode::State::QE:
		case SearchNode::State::QT:
			if (!node->isExpandedAll()) {
				gennodes = MoveGenerator::genNocapMove(node, player.kyokumen);
			}
			break;
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
		Evaluator::evaluate(gennodes, player);
		std::sort(node->children.begin(), node->children.end(), [](SearchNode* a, SearchNode* b)->int {return a->eval < b->eval; });
	
		//今展開したノードから静止探索
		{
			const double qsmassmax = tree.getMQS();
			const double T_cq = tree.getTcQ();
			const unsigned failmax = 30u;
			unsigned failnum = 0;
			while (failnum < failmax && !node->isQSTerminal() && node->mass < qsmassmax) {
				SearchNode* qnode = node;
				SearchPlayer qplayer = player;
				std::vector<SearchNode*> qhistory = { qnode };
				//選択
				while (!qnode->isNotExpanded()) {
					double emin = std::numeric_limits<double>::max();
					std::vector<dn> evals;
					for (auto child : qnode->children) {
						if (!child->isQSTerminal()) {
							const double eval = child->getEvaluation();
							evals.emplace_back(std::make_pair(eval, child));
							if (eval < emin) {
								emin = eval;
							}
						}
					}
					if (evals.empty()) {
						qnode->state = SearchNode::State::QT;
						goto qslooptail;
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
						if (qnode->move.isOute()) {
							qnode->setMate();
							goto qbackup;
						}
						else {
							qnode->state = SearchNode::State::QT;
							failnum++;
							goto qslooptail;
						}
					}
					qnode->state = SearchNode::State::QE;
					//評価,展開ノードの評価値バックアップ
					Evaluator::evaluate(gennodes, qplayer);
					{
						double emin = std::numeric_limits<double>::max();
						std::vector<double> evals;
						for (const auto& child : qnode->children) {
							const double eval = child->getEvaluation();
							evals.push_back(eval);
							if (eval < emin) {
								emin = eval;
							}
						}
						double Z_e = 0;
						for (const auto& eval : evals) {
							Z_e += std::exp(-(eval - emin) / T_e);
						}
						double E = 0;
						for (const auto& eval : evals) {
							E -= eval * std::exp(-(eval - emin) / T_e) / Z_e;
						}
						qnode->setEvaluation(E);
						qnode->setMass(1);
					}
				}
				//バックアップ
				qbackup:
				for (int i = qhistory.size() - 2; i >= 0; i--) {
					qnode = qhistory[i];
					//もし途中でLEノードが詰みになってしまったら、そのノードをフル展開する
					double emin = std::numeric_limits<double>::max();
					std::vector<dd> emvec;
					for (const auto child : qnode->children) {
						const double eval = child->eval;
						const double mass = child->mass;
						emvec.push_back(std::make_pair(eval, mass));
						if (eval < emin) {
							emin = eval;
						}
					}
					if (emin >= MateScoreBound) {
						if (qnode->isExpandedAll()) {
							qnode->setMateVariation(emin);
							continue;
						}
						else {
							SearchPlayer qp2(player);
							for (size_t j = 1; j <= i; j++) {
								qp2.proceed(qhistory[j]->move);
							}
							std::vector<SearchNode*> gennodes;
							gennodes = MoveGenerator::genNocapMove(qnode, qp2.kyokumen);
							if (gennodes.empty()) {
								qnode->setMateVariation(emin);
								continue;
							}
							else {
								Evaluator::evaluate(gennodes, qp2);
								for (const auto n : gennodes) {
									const double eval = n->eval;
									const double mass = n->mass;
									emvec.push_back(std::make_pair(eval, mass));
									if (eval < emin) {
										emin = eval;
									}
								}
							}
						}
					}
					else if (emin <= -MateScoreBound) {
						qnode->setMateVariation(emin);
						continue;
					}
					double Z_e = 0;
					double Z_d = 0;
					for (const auto em : emvec) {
						Z_e += std::exp(-(em.first - emin) / T_e);
						Z_d += std::exp(-(em.first - emin) / T_d);
					}
					double E = 0;
					double M = 1;
					for (const auto& em : emvec) {
						E -= em.first * std::exp(-(em.first - emin) / T_e) / Z_e;
						M += em.second * std::exp(-(em.first - emin) / T_d) / Z_d;
					}
					qnode->setEvaluation(E);
					qnode->setMass(M);
				}
			qslooptail:;
			}//静止探索1ループここまで

		}//静止探索ここまで
		node->state = SearchNode::State::E;
	}//展開評価ここまで

	//バックアップ
	backup:
	tree.excludeLeafNode(node);
	{
	for (int i = history.size() - 2; i >= 0; i--) {
			node = history[i];
			double emin = std::numeric_limits<double>::max();
			std::vector<dd> emvec;
			for (const auto& child : node->children) {
				const double eval = child->eval;
				const double mass = child->mass;
				emvec.push_back(std::make_pair(eval, mass));
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
				for (const auto& em : emvec) {
					const double eval = em.first;
					Z_e += std::exp(-(eval - emin) / T_e);
					Z_d += std::exp(-(eval - emin) / T_d);
				}
				double E = 0;
				double M = 1;
				for (const auto& em : emvec) {
					const double eval = em.first;
					const double mass = em.second;
					E -= eval * std::exp(-(eval - emin) / T_e) / Z_e;
					M += mass * std::exp(-(eval - emin) / T_d) / Z_d;
				}
				node->setEvaluation(E);
				node->setMass(M);
			}
		}
	}

	return newnodecount;
}
