#include "learn_util.h"
#include <iostream>

double LearnUtil::pTb = 0.1;//評価関数をシグモイド関数に写像した場合の方策の温度は、(元の温度)*(写像先の標準的な評価値差)/(元の標準的な評価値差)で求められる。

double LearnUtil::getChildrenZ(const SearchNode* const parent, const double T, double& CE) {
	if (parent->children.empty()) return 1;
	CE = std::numeric_limits<double>::max();
	for (const auto& child : parent->children) {
		CE = std::min(CE, child.eval.load());
	}
	double Z = 0;
	for (const auto& child : parent->children) {
		Z += std::exp(-(child.eval - CE) / T);
	}
	return Z;
}

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

SearchNode* LearnUtil::getPrincipalLeaf(SearchNode* const root) {
	const auto child = choiceBestChild(root);
	if (child == nullptr)
		return root;
	else if (child->isLeaf())
		return child;
	else
		return getPrincipalLeaf(child);
}

double alphabeta(Move& pmove, SearchPlayer& player, int depth, double alpha, double beta, SearchPlayer& bestplayer) {
	const auto eval = Evaluator::evaluate(player);
	if (depth <= 0) {
		return eval;
	}
	if (eval > alpha) {
		alpha = eval;
	}
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
		SearchPlayer cbestplayer = player;
		const double ceval = -alphabeta(m, player, depth - 1, -beta, -alpha, cbestplayer);
		if (ceval > alpha) {
			alpha = ceval;
			bestplayer = cbestplayer;
		}
		player.recede(m, captured, cache);
		if (alpha >= beta) {
			return alpha;
		}
	}
	return alpha;
}

SearchPlayer LearnUtil::getQSBest(const SearchNode* const root, SearchPlayer& player, const int depthlimit) {
	SearchPlayer bestplayer = player;
	if (depthlimit <= 0) {
		return player;
	}
	Move m(root->move);
	auto moves = MoveGenerator::genCapMove(m, player.kyokumen);
	if (moves.empty()) {
		return player;
	}
	double max = root->eval;
	for (auto m : moves) {
		const FeatureCache cache = player.feature.getCache();
		const koma::Koma captured = player.proceed(m);
		SearchPlayer cbestplayer = player;
		const double eval = -alphabeta(m, player, depthlimit - 1, std::numeric_limits<double>::lowest(), -max, cbestplayer);
		if (eval > max) {
			max = eval;
			bestplayer = cbestplayer;
		}
		player.recede(m, captured, cache);
	}
	return bestplayer;
}

SearchPlayer LearnUtil::getQSBest(const SearchNode* const root, SearchPlayer& player) {
	return getQSBest(root, player, SearchNode::getQSdepth());
}

double LearnUtil::EvalToProb(const double eval) {
	return 1.0 / (1.0 + std::exp(-eval / probT));
}

double LearnUtil::EvalToSignProb(const double eval) {
	return 2.0 / (1.0 + std::exp(-eval / probT)) - 1.0;
}

double LearnUtil::BackProb(const SearchNode& parent, const SearchNode& child, const double T) {
	double Z = 0;
	for (const auto& c : parent.children) {
		Z += std::exp(-(c.eval + parent.eval) / T);
	}
	return std::exp(-(child.eval + parent.eval) / T) / Z;
}

double LearnUtil::ResultToProb(GameResult result, bool teban) {
	switch (result)
	{
		case GameResult::SenteWin:
			return teban ? 1.0 : 0.0;
		case GameResult::GoteWin:
			return teban ? 0.0 : 1.0;
		case GameResult::Draw:
		default:
			return 0.5;
	}
}

double LearnUtil::ResultToReward(const GameResult result, const bool teban, const double win, const double draw, const double lose) {
	switch (result)
	{
		case GameResult::SenteWin:
			return teban ? win : lose;
		case GameResult::GoteWin:
			return teban ? lose : win;
		case GameResult::Draw:
		default:
			return draw;
	}
}

double LearnUtil::ResultToProb(MyGameResult result) {
	switch (result)
	{
		case MyGameResult::PlayerWin:
			return 1.0;
		case MyGameResult::PlayerLose:
			return 0;
		case MyGameResult::Draw: default:
			return 0.5;
	}
}

double LearnUtil::ResultToReward(const MyGameResult result, const double win, const double draw, const double lose) {
	switch (result)
	{
		case MyGameResult::PlayerWin:
			return win;
		case MyGameResult::PlayerLose:
			return lose;
		case MyGameResult::Draw: default:
			return draw;
	}
}

std::string LearnUtil::ResultToString(const GameResult& result) {
	switch (result)
	{
		case GameResult::SenteWin:
			return "sw";
		case GameResult::GoteWin:
			return "gw";
		case GameResult::Draw:
		default:
			return "draw";
	}
}

std::string LearnUtil::ResultToString(const MyGameResult& result) {
	switch (result)
	{
		case MyGameResult::PlayerWin:
			return "win";
		case MyGameResult::PlayerLose:
			return "lose";
		case MyGameResult::Draw: default:
			return "draw";
	}
}

double LearnUtil::change_evalTs_to_probTs(const double T) {
	return T * 0.2 / 500;
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