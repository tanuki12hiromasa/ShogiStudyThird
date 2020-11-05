#include "stdafx.h"
#include "learn_util.h"
#include <iostream>

double LearnUtil::pTb = 0.1;//評価関数をシグモイド関数に写像した場合の方策の温度は、(元の温度)*(写像先の標準的な評価値差)/(元の標準的な評価値差)で求められる。

SearchNode* LearnUtil::choiceChildRandom(const SearchNode* const root, const double T, double pip) {
	using dn = std::pair<double, SearchNode*>;
	double CE = std::numeric_limits<double>::max();
	if (root->isLeaf()) return nullptr;
	std::vector<dn> evals; evals.reserve(root->children.size());
	for (const auto& child : root->children) {
		const double eval = child->eval;
		evals.push_back(std::make_pair(eval, child));
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

SearchNode* LearnUtil::choiceBestChild(const SearchNode* const root) {
	double min = std::numeric_limits<double>::max();
	SearchNode* best = nullptr;
	for (const auto& child : root->children) {
		double eval = child->getEs();
		if (eval < min) {
			min = eval;
			best = child;
		}
	}
	return best;
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

LearnVec LearnUtil::getGrad(const SearchNode* const root, const SearchPlayer& rootplayer, bool teban, unsigned long long samplingnum, const int qsdepth) {
	const double T = 120;
	LearnVec vec;
	if (root == nullptr) return vec;
	if (root->children.empty()) {
		auto player = rootplayer;
		double c = 1;
		const auto qsbest = getQSBest(root, player, qsdepth);
		if (qsbest.kyokumen.teban() != player.kyokumen.teban()) c = -c;
		vec.addGrad(c, qsbest);
		return vec;
	}
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	for (unsigned long long i = 0; i < samplingnum; i++) {
		const SearchNode* node = root;
		double c = 1;
		double Peval_prev = (teban == rootplayer.kyokumen.teban()) ? EvalToProb(node->eval) : (1 - EvalToProb(node->eval));
		SearchPlayer player = rootplayer;
		while (!node->isLeaf()) {
			auto next = choiceChildRandom(node, T, random(engine));
			if (next == nullptr)break;
			node = next;
			player.proceed(node->move);
			const double Peval = (teban == player.kyokumen.teban()) ? EvalToProb(node->eval) : (1 - EvalToProb(node->eval));
			c *= -((Peval - Peval_prev) / pTb + 1);
			Peval_prev = Peval;
		}
		/*const auto qsbest = getQSBest(node, player, qsdepth);
		const double Peval = EvalToProb(Evaluator::evaluate(qsbest));
		c *= probT * Peval * (1 - Peval);
		if (qsbest.kyokumen.teban() != player.kyokumen.teban()) c = -c;
		vec.addGrad(c, qsbest, teban);*/
		const double Peval = EvalToProb(Evaluator::evaluate(player));
		c *= probT * Peval * (1 - Peval);
		vec.addGrad(c, player);
	}
	vec *= (1.0 / samplingnum);
	return vec;
}

LearnVec LearnUtil::getSamplingGradV(const SearchNode* const root, const SearchPlayer& rootplayer, const unsigned samplingnum) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	const double T = SearchNode::getTeval();
	LearnVec vec;
	if (!root)return vec;
	if (root->children.empty() || root->isLeaf()) {
		vec.addGrad(1, rootplayer);
		return vec;
	}
	for (unsigned count = 0; count < samplingnum; count++) {
		const SearchNode* node = root;
		SearchPlayer player = rootplayer;
		double c = 1;
		while (true) {
			if (node->children.empty() || node->isLeaf()) {
				const double sigH = EvalToProb(node->eval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			SearchNode* enode = choiceChildRandom(node, T, random(engine));
			if (!enode || enode->children.empty() || enode->isLeaf()) {
				const double sigH = EvalToProb(node->eval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			const double V = EvalToProb(node->eval);
			const double Q = 1 - EvalToProb(enode->eval);
			c *= (Q - V) / pTb + 1;
			const double fnodeeval = node->eval;
			node = choiceChildRandom(enode, T, random(engine));
			if (!node) {
				const double sigH = EvalToProb(fnodeeval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			player.proceed(enode->move);
			player.proceed(node->move);
		}
	}
	vec *= 1.0 / samplingnum;
	return vec;
}

LearnVec LearnUtil::getSamplingGradQ(const SearchNode* root, const SearchPlayer& rootplayer, const unsigned samplingnum) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	const double T = SearchNode::getTeval();
	LearnVec vec;
	if (!root)return vec;
	if (root->children.empty() || root->isLeaf()) {
		vec.addGrad(-1, rootplayer);
		return vec;
	}
	for (unsigned count = 0; count < samplingnum; count++) {
		const SearchNode* node = root;
		SearchPlayer player = rootplayer;
		double c = 1;
		while (true) {
			SearchNode* fnode = choiceChildRandom(node, T, random(engine));
			if (!fnode) break;
			player.proceed(fnode->move);
			if (fnode->children.empty() || fnode->isLeaf()) {
				const double sigH = EvalToProb(fnode->eval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			node = choiceChildRandom(fnode, T, random(engine));
			if (!node || node->children.empty() || node->isLeaf()) {
				const double sigH = EvalToProb(fnode->eval);
				c *= probT * sigH * (1 - sigH);
				vec.addGrad(c, player);
				break;
			}
			const double V = EvalToProb(fnode->eval);
			const double Q = 1 - EvalToProb(node->eval);
			c *= (Q - V) / pTb + 1;
			player.proceed(node->move);
		}
	}
	vec *= 1.0 / samplingnum;
	return vec;
}

double LearnUtil::EvalToProb(const double eval) {
	return 1.0 / (1.0 + std::exp(-eval / probT));
}