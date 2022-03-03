#pragma once
#include "agent.h"
#include "kppt_learn.h"

#ifdef USE_KPPT
using LearnVec = kppt::kppt_paramVector;
using LearnOpt = kppt::Adam;
#endif

#ifdef USE_NNUE
using LearnVec = Eval::NNUE::nnue_paramVector;
#endif

enum class GameResult {
	SenteWin, GoteWin, Draw
};

enum class MyGameResult {
	PlayerWin, PlayerLose, Draw
};

class LearnUtil {
public:
	static double getChildrenZ(const SearchNode* const parent, const double T, double& CE);
	static SearchNode* choicePolicyRandomChild(const SearchNode* const root, const double T, double pip);
	static SearchNode* choiceRandomChild(const SearchNode* const root, double pip);
	static SearchNode* choiceBestChild(const SearchNode* const root);
	static SearchNode* getPrincipalLeaf(const SearchNode* const root);
	static SearchPlayer getQSBest(const SearchNode* const root, SearchPlayer& player, const int depthlimit);
	static SearchPlayer getQSBest(const SearchNode* const root, SearchPlayer& player);
	static double EvalToProb(const double eval);
	static double EvalToSignProb(const double eval);
	static double BackProb(const SearchNode& parent, const SearchNode& child, const double T);
	static double ResultToProb(GameResult result, bool teban);
	static double ResultToReward(GameResult result, bool teban, double r_win, double r_draw, double r_lose);
	static double ResultToProb(MyGameResult result);
	static double ResultToReward(MyGameResult result, double r_win, double r_draw, double r_lose);
	static std::string ResultToString(const GameResult& result);
	static std::string ResultToString(const MyGameResult& result);
	static constexpr double probT = 600.0;
	static double pTb;
	static double change_evalTs_to_probTs(const double evalT);
	static std::string getDateString();
};
