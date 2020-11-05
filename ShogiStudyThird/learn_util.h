#pragma once
#include "agent.h"
#include "apery_learn.h"

#ifdef USE_KPPT
using LearnVec = apery::apery_paramVector;
#endif


class LearnUtil {
public:
	static SearchNode* choiceChildRandom(const SearchNode* const root, const double T, double pip);
	static SearchNode* choiceBestChild(const SearchNode* const root);
	static SearchPlayer getQSBest(const SearchNode* const root, SearchPlayer& player, const int depthlimit);
	static LearnVec getGrad(const SearchNode* const root, const SearchPlayer& player, bool teban, unsigned long long samplingnum, const int qsdepth = 8);
	static LearnVec getSamplingGrad(const SearchNode* root, const SearchPlayer& player, bool teban, unsigned long long samplingnum, const int qsdepth = 8);
	static LearnVec getSamplingGradV(const SearchNode* root, const SearchPlayer& player, unsigned samplingnum);
	static LearnVec getSamplingGradQ(const SearchNode* root, const SearchPlayer& player, unsigned samplingnum);
	static double EvalToProb(const double eval);
	static constexpr double probT = 600.0;
	static double pTb;
};