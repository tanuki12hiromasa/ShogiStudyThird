#pragma once
#include "commander.h"
#include "learn_util.h"

#define SAMPLING_GRAD_PROB //サンプリングによる勾配ベクトル
//#define SAMPLING_GRAD_CP
//#define PVLEAF_GRAD //最善応手手順の末端ノードによる勾配ベクトル

#define LEARN_TREE_LEAF //末端ノードの局面で学習
//#define LEARN_QS_LEAF //末端ノードから静止探索した局面で学習

#define LEARN_TD_LAMBDA_CP
//#define LEARN_TD_LAMBDA_PROB
#define LEARN_PG
#define LEARN_REGRESSION
#define LEARN_BOOTSTRAP_ROOT
#define LEARN_BOOTSTRAP_RANDOM_NODE

#if defined(SAMPLING_GRAD_PROB) || defined(SAMPLING_GRAD_CP)
#define SAMPLING_GRAD
#endif

#if defined(LEARN_TD_LAMBDA_PROB) || defined(LEARN_TD_LAMBDA_CP)
#define LEARN_TD_LAMBDA
#endif
#if defined(LEARN_TD_LAMBDA) || defined(LEARN_REGRESSION)
#define LEARN_V
#endif

class LearnCommander : Commander {

public:
	static void execute();

protected:
	static void coutID();
	static void coutLearnerOption();
	void setLearnerOption(const std::vector<std::string>& token);
	void gameInit();

	static bool loadTempInfo(int& learncount, std::string& datestr, const std::string& dir = "./learning/");
	static bool saveTempInfo(const int& learncount, const std::string& datestr, const std::string& dir = "./learning/");
	static bool LoadTempVec(LearnVec& vec, const int vecnum = 1, const std::string& dir = "./learning/");
	static bool SaveTempVec(LearnVec& vec, const int vecnum = 1, const std::string& dir = "./learning/");

	void go(const std::vector<std::string>& tokens);
	void gameover(const std::vector<std::string>& tokens);

	void learn_from_tree(const SearchPlayer& kyokumen, const SearchNode* const root, const SearchNode* const bestmove);
	void learn_at_gameover(MyGameResult result);

	bool quit = false;
	Random::xoshiro256p random;

	int batch = 1;
	double T_search = 120;
	double kifulength_mean = 30;

	bool learned_gameover = false;

#ifdef SAMPLING_GRAD
	long long unsigned samplingnum = 10000;
	double T_sgp = 1.0;
#endif

#ifdef LEARN_TD_LAMBDA
	double td_rate = 0.01;
	double td_lambda = 0.9;
	double td_gamma = 0.95;

	LearnVec dw_td, td_e_vec;
	double td_Vt = 0.5;
	bool td_first = true;
#endif

#ifdef LEARN_REGRESSION
	double reg_rate = 0.01;
	LearnVec dw_reg_win, dw_reg_lose;
#endif

#ifdef LEARN_PG
	double pg_rate = 0.01;
	LearnVec dw_pg;
#endif

#ifdef LEARN_BOOTSTRAP_ROOT
	double btsroot_rate = 0.001;
	LearnVec dw_btsroot;
#endif
#ifdef LEARN_BOOTSTRAP_RANDOM_NODE
	double btsrand_rate = 0.01;
	double btsrand_exp = 0.0001;
	LearnVec dw_btsrand;
#endif

	double reward_win = 1.0;
	double reward_lose = -1.0;
	double reward_draw = 0;
};