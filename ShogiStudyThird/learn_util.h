#pragma once
#include "agent.h"
#include "kppt_learn.h"

#ifdef  USE_KPPT
using LearnVec = kppt::kppt_paramVector;
#endif //  USE_KPPT


class LearnUtil {
public:
	static SearchNode* choicePolicyRandomChild(const SearchNode* const root, const double T, double pip);
	static SearchNode* choiceRandomChild(const SearchNode* const root, double pip);
	static SearchNode* choiceBestChild(const SearchNode* const root);
	static SearchNode* getPrincipalLeaf(const SearchNode* const root);
	static double EvalToProb(const double eval);
	static double BackProb(const SearchNode& parent, const SearchNode& child, const double T);
	static constexpr double probT = 600.0;
	static double pTb;
};

enum class GameResult {
	SenteWin, GoteWin, Draw
};