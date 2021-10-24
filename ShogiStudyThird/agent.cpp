#include "stdafx.h"
#include "agent.h"
#include "leaf_guard.h"
#include <algorithm>

bool SearchAgent::leave_QsearchNode = false;
bool SearchAgent::use_original_kyokumen_eval = false;
bool SearchAgent::QS_relativeDepth = false;
int SearchAgent::drawmovenum = 320;

std::atomic_bool SearchAgent::search_enable;
std::atomic_uint64_t SearchAgent::time_stamp;
std::atomic_uint SearchAgent::old_threads_num;


SearchAgent::SearchAgent(SearchTree& tree, const double Ts,const Random::xoshiro256p& seed)
	:tree(tree),random(seed),Ts(Ts)
{
	status = state::search;
	searching = false;
	th = std::thread(&SearchAgent::loop, this);
}

SearchAgent::SearchAgent(SearchAgent&& agent) noexcept
	: tree(agent.tree), th(std::move(agent.th)), Ts(agent.Ts), random(std::move(agent.random))
{
	status = agent.status.load();
	searching = agent.searching.load();
}

SearchAgent::~SearchAgent() {
	status = state::terminate;
	if (th.joinable()) th.join();
}


void SearchAgent::loop() {
	using namespace std::chrono_literals;
	std::uint64_t mystamp = 0;
	while (status != state::terminate) {
		if (old_threads_num > 0 && time_stamp != mystamp) {
			old_threads_num--;
			mystamp = time_stamp;
		}
		switch (status) {
			case state::search: {
				if (search_enable) simulate(tree.getRoot());
				else std::this_thread::sleep_for(20ms);
				break;
			}
			case state::gc: {
				bool ok = deleteGarbage();
				if (ok) status = state::search;
				break;
			}
		}
	}
}

//探索中かどうかを判定するフラグを管理するクラス オブジェクトが破棄される時に自動でフラグを戻してくれるので戻し忘れが無くて便利
struct Searching {
	Searching(std::atomic_bool& searching) :searching(searching) { searching = true; }
	~Searching() { searching = false; }
	std::atomic_bool& searching;
};

void SearchAgent::simulate(SearchNode* const root) {
	using dn = std::pair<double, SearchNode*>;
	using dd = std::pair<double, double>;
	Searching _searching(searching);
	const double T_e = SearchTemperature::Te;
	const double T_d = SearchTemperature::Td;
	const double MateScoreBound = SearchNode::getMateScoreBound();
	SearchNode* node = root;
	player = tree.getRootPlayer();
	std::vector<SearchNode*> history = { node };
	std::vector<std::pair<uint64_t, std::array<uint8_t, 95>>> k_history;
	//選択
	while (!node->isLeaf()) {
		if (!search_enable) return;
		double CE = std::numeric_limits<double>::max();
		std::vector<dn> evals; evals.reserve(node->children.size());
		for (auto& child : node->children) {
			if (child.isSearchable()) {
				double eval = child.getEs();
				evals.push_back(std::make_pair(eval,&child));
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
			return;
		}
		const double T_c = SearchTemperature::getTs_atNode(Ts, *node);
		double Z = 0;
		for (const auto& eval : evals) {
			Z += std::exp(-(eval.first - CE) / T_c);
		}
		double pip = Z * random.rand01();
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
		k_history.emplace_back(player.kyokumen.getHash(), player.kyokumen.getBammen());
		history.push_back(node);
	}
	//展開・評価
	{
		if (!search_enable) return;
		//末端ノードが他スレッドで展開中になっていないかチェック
		LeafGuard dredear(node);
		if (!dredear.Result()) {
			return;
		}
		/*
		if (player.kyokumen.isDeclarable()) {
			node->setDeclare();
			goto backup;
		}
		*/
		
		{//子ノード生成
			const auto moves = MoveGenerator::genMove(node->move, player.kyokumen);
			if (moves.empty()) {
				node->setMate();
				goto backup;
			}
			else if (history.size() - 1 + tree.getMoveNum() >= drawmovenum) {
				node->setRepetition(player.kyokumen.teban());
				goto backup;
			}
			node->addChildren(moves);
		}
#if 0
#else
		uint64_t evalcount = 0ull;
		for (auto& child : node->children) {
			const auto cache = player.proceedC(child.move);
			history.pop_back();
			if (!checkRepetition(&child, player.kyokumen, history, k_history)) {
				evalcount += qsimulate(&child, player, history.size());
			}
			history.pop_back();
			player.recede(child.move, cache);
		}
		tree.addEvaluationCount(evalcount);
		//sortは静止探索後の方が評価値順の並びが維持されやすい　親スタートの静止探索ならその前後共にsortしてもいいかもしれない
		node->children.sort();
		//sortしたのでfrontが最小値になっているはず
		double emin = node->children.begin()->eval;
		double Z_e = 0;
		for (const auto& child : node->children) {
			Z_e += std::exp(-(child.eval - emin) / T_e);
		}
		double E = 0;
		for (const auto& child : node->children) {
			E -= child.eval * std::exp(-(child.eval - emin) / T_e) / Z_e;
		}
		node->setEvaluation(E);
		node->setMass(1);
		//node->status = SearchNode::State::E;
#endif
	}//展開評価ここまで

	//バックアップ
	backup:
	{
		for (int i = history.size() - 2; i >= 0; i--) {
			node = history[i];
			double emin = std::numeric_limits<double>::max();
			std::vector<dd> emvec; emvec.reserve(node->children.size());
			for (const auto& child : node->children) {
				const double eval = child.getEvaluation();
				const double mass = child.mass;
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
		const FeatureCache cache = player.feature.getCache();
		const koma::Koma captured = player.proceed(m);
		alpha = std::max(-alphabeta(m, player, depth - 1, -beta, -alpha, evalcount), alpha);
		player.recede(m, captured, cache);
		if (alpha >= beta) {
			return alpha;
		}
	}
	return alpha;
}

size_t SearchAgent::qsimulate(SearchNode* const root, SearchPlayer& player, const int hislength) {
	const int depth = (QS_relativeDepth) ? (SearchNode::getQSdepth() - hislength) : SearchNode::getQSdepth();
	if (player.kyokumen.isDeclarable()) {
		root->setDeclare();
		return 0;
	}
	auto moves = MoveGenerator::genCapMove(root->move, player.kyokumen);
	if (moves.empty()) {
		if (root->move.isOute()) {
			root->setMate();
			return 1ull;
		}
		else {
			const double eval = Evaluator::evaluate(player);
			root->setEvaluation(eval);
			root->setOriginEval(eval);
			return 1ull;
		}
	}
	else if (hislength - 1 + tree.getMoveNum() >= drawmovenum) {
		root->setRepetition(player.kyokumen.teban());
		return 1ull;
	}
	if (depth <= 0) {
		const double eval = Evaluator::evaluate(player);
		root->setEvaluation(eval);
		root->setOriginEval(eval);
		return 1ull;
	}
	double max = Evaluator::evaluate(player);
	uint64_t evaluationcount = 1ull;
	for (auto m : moves) {
		const FeatureCache cache = player.feature.getCache();
		const koma::Koma captured = player.proceed(m);
		const double eval = -alphabeta(m, player, depth - 1, std::numeric_limits<double>::lowest(), -max, evaluationcount);
		if (eval > max) {
			max = eval;
		}
		player.recede(m, captured, cache);
	}
	root->setEvaluation(max);
	if (use_original_kyokumen_eval) root->setOriginEval(Evaluator::evaluate(player));
	else root->setOriginEval(max);
	return evaluationcount;
}

bool SearchAgent::checkRepetition(SearchNode* const node, const Kyokumen& kyokumen, const std::vector<SearchNode*>& history, const std::vector<std::pair<std::uint64_t, Bammen>>& k_history) {
	unsigned repnum = 0;
	SearchNode* repnode = nullptr;
	SearchNode* latestRepnode = nullptr;

	auto p = tree.findRepetition(kyokumen);
	repnum += p.first;
	repnode = p.second;
	const auto hash = kyokumen.getHash();
	//先後一致している方のみを調べればよいので1つ飛ばしで調べる
	for (int t = k_history.size() - 2; t >= 0; t -= 2) {
		const auto& k = k_history[t];
		if (k.first == hash && k.second == kyokumen.getBammen()) {
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

	if (repnum > 0/*千日手である*/) {
		if (repnum >= 3) {
			//同一局面が4回繰り返されているなら、ゲーム終了
			if (checkRepetitiveCheck(kyokumen, history, latestRepnode)) {
				node->setRepetitiveCheckmate();
			}
			else {
				node->setRepetitiveEnd(kyokumen.teban());
			}
		}
		else {
			//繰り返し回数が少ない場合は、評価値を千日手にする
			if (checkRepetitiveCheck(kyokumen, history, latestRepnode)) {
				node->setRepetitiveCheck();
			}
			else {
				node->setRepetition(kyokumen.teban());
			}
		}
		return true;
	}
	return false;
}

bool SearchAgent::checkRepetitiveCheck(const Kyokumen& kyokumen,const std::vector<SearchNode*>& searchhis, const SearchNode* const repnode)const {
	//対象ノードは未展開なので局面から王手かどうか判断する この関数は4回目の繰り返しの終端ノードで呼ばれているのでkusemonoを何回も生成するような無駄は発生していない
	if (!searchhis.back()->move.isOute()) {
		if (!kyokumen.isOute(searchhis.back()->move)) {
			return false;
		}
		searchhis.back()->move.setOute(true);
	}
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

//削除が完了したらtrueを返す
bool SearchAgent::deleteGarbage() {
	using namespace std::chrono_literals;
	if (old_threads_num > 0) {
		std::this_thread::sleep_for(10ms);
		return false;
	}
	else {
		return !tree.deleteGarbage();
	}
}

AgentPool::AgentPool(SearchTree& tree):tree(tree) {
	SearchAgent::old_threads_num = 0;
	SearchAgent::search_enable = false;
	SearchAgent::time_stamp = std::random_device()();
}

void AgentPool::setup() {
	assert(!SearchAgent::search_enable);
	Random::xoshiro256p random;
	if (agents.size() < agent_num) {
		for (std::size_t t = agents.size(); t < agent_num; t++) {
			agents.push_back(std::unique_ptr<SearchAgent>(
				new SearchAgent(tree, SearchTemperature::getTs((double)t / (agent_num - 1)), random)));
			random.jump();
		}
	}
	else if (agents.size() > agent_num) {
		for (std::size_t t = agent_num; t < agents.size(); t++) {
			agents[t]->status = SearchAgent::state::terminate;
		}
		agents.erase(agents.begin() + agent_num, agents.end());
	}
}

void AgentPool::startSearch() {
	SearchAgent::search_enable = true;
}

void AgentPool::pauseSearch() {
	SearchAgent::search_enable = false;
}

void AgentPool::joinPause() {
	using namespace std::chrono_literals;
	SearchAgent::search_enable = false;
	while (true) {
		bool allpause = true;
		for (const auto& agent : agents) if (agent->searching) allpause = false;
		if (allpause) return;
		std::this_thread::sleep_for(20ms);
	}
}

void AgentPool::noticeProceed() {
	assert(!SearchAgent::search_enable);
	if (agent_num <= gc_num) return;
	for (std::size_t t = agent_num - gc_num; t < agent_num; t++) {
		const auto& agent = agents[t];
		agent->status = SearchAgent::state::gc;
	}
	SearchAgent::old_threads_num = agent_num;
	SearchAgent::time_stamp++;
}

void AgentPool::deleteTree() {
	using namespace std::chrono_literals;
	SearchAgent::search_enable = false;
	tree.makeNewTree(Kyokumen(), {});
	for (const auto& agent : agents) {
		agent->status = SearchAgent::state::gc;
	}
	while (!tree.deleteGarbage()) { std::this_thread::sleep_for(20ms); }
}

void AgentPool::terminate() {
	joinPause();
	for (const auto& agent : agents) {
		agent->status = SearchAgent::state::terminate;
	}
	agents.clear();
	agent_num = 0;
}