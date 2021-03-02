#include "stdafx.h"
#include "learn_method.h"
#include <iostream>
#include <set>

void RootStrap::update(SearchNode* const root, const SearchPlayer& rootplayer) {
	const double H = Evaluator::evaluate(rootplayer);
	const double sigH = LearnUtil::EvalToProb(H);
	const double c = rate * -(sigH - LearnUtil::EvalToProb(root->eval))
		* LearnUtil::probT * sigH * (1 - sigH);
	dw.addGrad(c, rootplayer);
	std::cout << H << " -> " << root->eval << "\n";
}

void SamplingBTS::update(SearchNode* const root, const SearchPlayer& rootplayer) {
	std::set<SearchNode*> learned_nodes;
	for (size_t t = 0; t < sampling_num; t++) {
		auto player = rootplayer;
		auto node = root;
		double realization = 1.0;
		while (node) {
			if (!learned_nodes.contains(node)) {
				learned_nodes.insert(node);

				const double H = Evaluator::evaluate(rootplayer);
				const double sigH = LearnUtil::EvalToProb(H);
				const double c = rate * std::sqrt(realization) * -(sigH - LearnUtil::EvalToProb(root->eval))
					* LearnUtil::probT * sigH * (1 - sigH);
				dw.addGrad(c, rootplayer);
			}
			if (node->children.empty() || node->isLeaf()) { break; }
			const auto next = LearnUtil::choicePolicyRandomChild(node, T, random(engine));
			if (next == nullptr) { break; }
			player.proceed(next->move);
			realization *= LearnUtil::BackProb(*node, *next, T);
			node = next;
		}
	}
	std::cout << "learned kyokumen:" << learned_nodes.size() << "\n";
	samplingPosNum += learned_nodes.size();
}

void SamplingBTS::fin(SearchNode* const root, const SearchPlayer& player, GameResult result) {
	switch (result)
	{
		case GameResult::SenteWin:
		case GameResult::GoteWin:
			if (!root->children.empty()) update(root, player);
			break;
		case GameResult::Draw:
		default:
			//引き分けは学習しない
			break;
	}
}