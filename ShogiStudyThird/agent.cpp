#include "stdafx.h"
#include "agent.h"
#include "leaf_guard.h"
#include <algorithm>

bool SearchAgent::leave_QsearchNode = false;
bool SearchAgent::use_original_kyokumen_eval = false;
bool SearchAgent::QS_relativeDepth = false;

std::atomic<size_t> SearchAgent::simCount(0);

SearchAgent::SearchAgent(SearchTree& tree, const double Ts,int seed)
	:tree(tree),engine(seed),root(tree.getRoot()),Ts(Ts)
{
	
	if (root != nullptr)
		alive = true;
	else 
		alive = false;
	th = std::thread(&SearchAgent::loop, this);
}

SearchAgent::SearchAgent(SearchAgent&& agent) noexcept
	: tree(agent.tree), th(std::move(agent.th)), Ts(agent.Ts),
	root(agent.root), engine(std::move(agent.engine))
{
	alive = agent.alive.load();
}



void SearchAgent::loop() {
	size_t newnodecount = 0;
	while (alive) {
		//if (simCount.load() < 10000) {
		//	++simCount;
			newnodecount = simulate(root);
			tree.addNodeCount(newnodecount);
		//}
	}
}

size_t SearchAgent::simulate(SearchNode* const root) {
	using dn = std::pair<double, SearchNode*>;
	using dd = std::pair<double, double>;
	const double T_e = SearchNode::getTeval();
	const double T_d = SearchNode::getTdepth();
	const double MateScoreBound = SearchNode::getMateScoreBound();
	size_t newnodecount = 0;
	SearchNode* node = root;
	player = tree.getRootPlayer();
	std::vector<SearchNode*> history = { node };
	std::vector<std::pair<uint64_t, std::array<uint8_t, 95>>> k_history;
	//選択
	while (!node->isLeaf()) {
		if (!alive) return 0;
		double CE = std::numeric_limits<double>::max();
		std::vector<dn> evals;
		for (const auto& child : node->children) {
			if (child->isSearchable()) {
				double eval = child->getEs();
				evals.push_back(std::make_pair(eval,child));
				if (eval < CE) {
					CE = eval;
				}
			}
		}
		if (evals.empty()) {
			//node->status = SearchNode::State::Terminal;
			if (history.size() == 1) {
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(200us);
			}
			return 0;
		}
		const double T_c = node->getTs(Ts);
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
		k_history.push_back(std::make_pair(player.kyokumen.getHash(), player.kyokumen.getBammen()));
		player.proceed(node->move);
		history.push_back(node);
	}
	//展開・評価
	{
		if (!alive) return 0;
		//末端ノードが他スレッドで展開中になっていないかチェック
		LeafGuard dredear(node);
		if (!dredear.Result()) {
			return 0;
		}
		if (player.kyokumen.isDeclarable()) {
			node->setDeclare();
			goto backup;
		}
		//千日手チェック
		unsigned repnum = 0;
		SearchNode* repnode = nullptr;
		SearchNode* latestRepnode = nullptr;
		{
			auto p = tree.findRepetition(player.kyokumen);
			repnum += p.first;
			repnode = p.second;
			const auto hash = player.kyokumen.getHash();
			//先後一致している方のみを調べればよいので1つ飛ばしで調べる
			for (int t = k_history.size() - 2; t >= 0; t -= 2) {
				const auto& k = k_history[t];
				if ( k.first == hash && k.second==player.kyokumen.getBammen()) {
					repnum++;
					repnode = history[t];
					if (latestRepnode == nullptr) {
						latestRepnode = repnode;
					}
				}
			}
			if (latestRepnode == nullptr) {
				latestRepnode = repnode;
			}
		}
		if (repnum > 0/*千日手である*/) {
			if (repnum >= 3) {
				if (checkRepetitiveCheck(player.kyokumen,history,latestRepnode)) {
					node->setRepetitiveCheck();
				}
				else {
					node->setRepetition(player.kyokumen.teban());
				}
				goto backup;
			}
			/*else if(!repnode->isLeaf()) {
				nodeCopy(repnode, node);
				goto backup;
			}*/
		}
		{//子ノード生成
			const auto moves = MoveGenerator::genMove(node->move, player.kyokumen);
			if (moves.empty()) {
				node->setMate();
				goto backup;
			}
			node->addChildren(moves);
			newnodecount += moves.size();
		}
		uint64_t evalcount = 0ull;
		for (auto child : node->children) {
			const auto cache = player.proceedC(child->move);
			evalcount += qsimulate(child, player, history.size());
			player.recede(child->move, cache);
		}
		tree.addEvaluationCount(evalcount);
		//sortは静止探索後の方が評価値順の並びが維持されやすい　親スタートの静止探索ならその前後共にsortしてもいいかもしれない
		std::sort(node->children.begin(), node->children.end(), [](SearchNode* a, SearchNode* b)->int {return a->eval < b->eval; });
		//sortしたのでfrontが最小値になっているはず
		double emin = node->children.front()->eval;
		double Z_e = 0;
		for (const auto& child : node->children) {
			Z_e += std::exp(-(child->eval - emin) / T_e);
		}
		double E = 0;
		for (const auto& child : node->children) {
			E -= child->eval * std::exp(-(child->eval - emin) / T_e) / Z_e;
		}
		node->setEvaluation(E);
		node->setMass(1);
		//node->status = SearchNode::State::E;
	}//展開評価ここまで

	//バックアップ
	backup:
	{
		for (int i = history.size() - 2; i >= 0; i--) {
			node = history[i];
			double emin = std::numeric_limits<double>::max();
			std::vector<dd> emvec;
			for (const auto& child : node->children) {
				const double eval = child->getEvaluation();
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

double alphabeta(Move& pmove,SearchPlayer& player, int depth, double alpha, double beta, uint64_t& evalcount) {
	evalcount++;
	const auto eval = Evaluator::evaluate(player);
	if (depth <= 0) {
		return eval;
	}
	alpha = std::max(eval, alpha);
	if (alpha >= beta) {
		return alpha;
	}
	auto moves = MoveGenerator::genCapMove(pmove, player.kyokumen);
	if (moves.empty() || pmove.isOute()) {
		return eval;
	}
	for (auto& m : moves) {
		const FeaureCache cache = player.feature.getCache();
		const koma::Koma captured = player.proceed(m);
		alpha = std::max(-alphabeta(m, player, depth - 1, -beta, -alpha, evalcount), alpha);
		player.recede(m, captured, cache);
		if (alpha >= beta) {
			return alpha;
		}
	}
	return alpha;
}

uint64_t SearchAgent::qsimulate(SearchNode* const root, SearchPlayer& p, const int hislength) {
	const int depth = (QS_relativeDepth) ? (SearchNode::getQSdepth() - hislength) : SearchNode::getQSdepth();
	if (depth <= 0) {
		const double eval = Evaluator::evaluate(p);
		root->setEvaluation(eval);
		root->setOriginEval(eval);
		return 1ull;
	}
	auto moves = MoveGenerator::genCapMove(root->move, player.kyokumen);
	if (moves.empty()) {
		if (root->move.isOute()) {
			root->setMate();
			return 1ull;
		}
		else {
			const double eval = Evaluator::evaluate(p);
			root->setEvaluation(eval);
			root->setOriginEval(eval);
			return 1ull;
		}
	}
	double max = Evaluator::evaluate(p);
	uint64_t evaluationcount = 1ull;
	for (auto m : moves) {
		const FeaureCache cache = player.feature.getCache();
		const koma::Koma captured = player.proceed(m);
		const double eval = -alphabeta(m, p, depth - 1, std::numeric_limits<double>::lowest(), -max, evaluationcount);
		if (eval > max) {
			max = eval;
		}
		player.recede(m, captured, cache);
	}
	root->setEvaluation(max);
	if (use_original_kyokumen_eval) root->setOriginEval(Evaluator::evaluate(p));
	else root->setOriginEval(max);
	return evaluationcount;
}

bool SearchAgent::checkRepetitiveCheck(const Kyokumen& kyokumen,const std::vector<SearchNode*>& searchhis, const SearchNode* const repnode)const {
	//対象ノードは未展開なので局面から王手かどうか判断する この関数は4回目の繰り返しの終端ノードで呼ばれているのでkusemonoを何回も生成するような無駄は発生していない
	if (kyokumen.teban()) {
		if (kyokumen.getSenteOuCheck().empty()) {
			return false;
		}
	}
	else {
		if (kyokumen.getGoteOuCheck().empty()) {
			return false;
		}
	}
	searchhis.back()->move.setOute(true);
	//過去ノードはmoveのouteフラグから王手だったか判定する
	int t;
	for (t = searchhis.size() - 3; t >= 0; t -= 2) {//historyの後端は末端ノードなのでその二つ前から調べていく
		if (!searchhis[t]->move.isOute()) {
			return false;
		}
		if (searchhis[t] == repnode) {
			return true;
		}
	}
	const auto& treehis = tree.getHistory();
	for (t += treehis.size() - 1; t >= 0; t -= 2) {
		if (!treehis[t]->move.isOute()) {
			return false;
		}
		if (treehis[t] == repnode) {
			return true;
		}
	}
	return false;
}

void SearchAgent::nodeCopy(const SearchNode* const origin, SearchNode* const copy)const {
	copy->setEvaluation(origin->getEvaluation());
	copy->move.setOute(origin->move.isOute());
	for (auto& child : origin->children) {
		copy->addCopyChild(child);
	}
	copy->setMass(1);
	//copy->status = SearchNode::State::E;
}