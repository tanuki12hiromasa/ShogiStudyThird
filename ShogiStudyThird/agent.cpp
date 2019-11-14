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
		std::vector<SearchNode*> gennodes;
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
			node->setMate();
			goto backup;
		}
		newnodecount += gennodes.size();
		//Evaluator::evaluate(gennodes, player);下のforループ内で評価するので不要になった
		for (auto child : node->children) {
			SearchPlayer p = player;
			p.proceed(child->move);
#if 0 //静止探索ノードを残す場合
			newnodecount += qsimulate(child, p);
			child->setMass(0);//静止探索の探索指標は別物なので0に戻す
#else //静止探索ノードを残さない場合
			qsimulate(child, p);
			child->setMass(0);//
			child->deleteTree();
#endif
		}
		//sortは静止探索後の方が評価値順の並びが維持されやすい　親スタートの静止探索ならその前後共にsortしてもいいかもしれない
		std::sort(node->children.begin(), node->children.end(), [](SearchNode* a, SearchNode* b)->int {return a->eval < b->eval; });
		
		
		node->state = SearchNode::State::E;
	}//展開評価ここまで

	//バックアップ
	backup:
	tree.excludeLeafNode(node);
	{
	for (int i = history.size() - 1; i >= 0; i--) {
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

size_t SearchAgent::qsimulate(SearchNode* const root, const SearchPlayer& p) {
	using dn = std::pair<double, SearchNode*>;
	using dd = std::pair<double, double>;
	size_t newnodecount = 0u;
	unsigned failnum = 0u;
	const unsigned failmax = 5u;
	const double Mmax = tree.getMQS();
	const double T_c = tree.getTcQ();
	const double T_d = tree.getTdepth();
	const double T_e = tree.getTeval();
	const double MateScoreBound = SearchNode::getMateScoreBound();
	while (root->isQSTerminal() && root->mass < Mmax && failnum < failmax) {
		SearchNode* node = root;
		SearchPlayer player = p;
		std::vector<SearchNode*> history = { node };
		//選択
		while (node->isNotExpanded()) {
			double emin = std::numeric_limits<double>::max();
			std::vector<dn> evals;
			for (auto child : node->children) {
				if (!child->isQSTerminal()) {
					const double eval = child->getEvaluation();
					evals.emplace_back(std::make_pair(eval, child));
					if (eval < emin) {
						emin = eval;
					}
				}
			}
			if (evals.empty()) {
				node->state = SearchNode::State::QT;
				failnum++;
				goto looptail;
			}
			double Z = 0;
			for (const auto& dn : evals) {
				Z += std::exp(-(dn.first - emin) / T_c);
			}
			double pip = Z * random(engine);
			node = evals.front().second;
			for (const auto& dn : evals) {
				pip -= std::exp(-(dn.first - emin) / T_c);
				if (pip <= 0) {
					node = dn.second;
					break;
				}
			}
			player.proceed(node->move);
			history.push_back(node);
		}
		//展開
		{
			if (player.kyokumen.isDeclarable()) {
				node->setDeclare();
				goto backup;
			}
			std::vector<SearchNode*> gennodes;
			gennodes = MoveGenerator::genCapMove(node, player.kyokumen);
			newnodecount += gennodes.size();
			if (gennodes.empty()) {
				if (node->isExpandedAll()) {
					node->setMate();
					goto backup;
				}
				else {
					node->state = SearchNode::State::QT;
					failnum++;
					goto looptail;
				}
			}
			node->state = SearchNode::State::QE;
			Evaluator::evaluate(gennodes, player);
			double emin = std::numeric_limits<double>::max();
			std::vector<double> evals;
			for (const auto& child : node->children) {
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
			node->setEvaluation(E);
			node->setMass(1);
		}
		backup:
		//バックアップ
		for (int i = history.size() - 2; i >= 0; i--) {
			node = history[i];
			//もし途中でLEノードが詰みになってしまったら、そのノードをフル展開する
			double emin = std::numeric_limits<double>::max();
			std::vector<dd> emvec;
			for (const auto child : node->children) {
				const double eval = child->eval;
				const double mass = child->mass;
				emvec.push_back(std::make_pair(eval, mass));
				if (eval < emin) {
					emin = eval;
				}
			}
			if (emin >= MateScoreBound) {
				if (node->isExpandedAll()) {
					node->setMateVariation(emin);
					continue;
				}
				else {
					SearchPlayer qp2(player);
					for (size_t j = 1; j <= i; j++) {
						qp2.proceed(history[j]->move);
					}
					std::vector<SearchNode*> gennodes;
					gennodes = MoveGenerator::genNocapMove(node, qp2.kyokumen);
					newnodecount += gennodes.size();
					if (gennodes.empty()) {
						node->setMateVariation(emin);
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
				node->setMateVariation(emin);
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
			node->setEvaluation(E);
			node->setMass(M);
		}
		newnodecount = 0u;
	looptail:;
	}
	if (root->children.empty()) {
		if (!root->isExpandedAll()){
			Evaluator::evaluate(root, p);
		}
	}
	return newnodecount;
}