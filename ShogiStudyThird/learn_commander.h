#pragma once
#include "commander.h"
#include "learn_util.h"

//#define SAMPLING_GRAD_PROB //2手ずつサンプリングする勾配ベクトル(評価値を[0,1]にする)
//#define SAMPLING_GRAD_CP //2手ずつサンプリングする勾配ベクトル(評価値そのまま)
#define SAMPLING_GRAD_STEP_PROB //1手ずつサンプリングする勾配ベクトル(評価値を[0,1]にする)
//#define SAMPLING_GRAD_STEP_CP //1手ずつサンプリングする勾配ベクトル(評価値そのまま)(未実装)
//#define PVLEAF_GRAD_SKIP //最善応手手順の末端ノード(PV-Leaf)による勾配ベクトル(自手番のみ)
//#define PVLEAF_GRAD_STEP //PV-Leafによる勾配ベクトル(相手番も含む)

//#define LEARN_TREE_LEAF //末端ノードの局面で学習
//#define LEARN_QS_LEAF //末端ノードから静止探索した局面で学習

//#define LEARN_TD_LAMBDA_CP
#define LEARN_TD_LAMBDA_PROB
//#define LEARN_TD_LAMBDA_ROOT
#define LEARN_PG
//#define LEARN_PG_ROOT
#define LEARN_REGRESSION_LEAVES
#define LEARN_REGRESSION_ROOT
#define LEARN_BOOTSTRAP_ROOT
#define LEARN_BOOTSTRAP_RANDOM_NODE

#if defined(SAMPLING_GRAD_PROB) || defined(SAMPLING_GRAD_CP)
#define SAMPLING_GRAD_SKIP
#endif
#if defined(SAMPLING_GRAD_STEP_PROB) || defined(SAMPING_GRAD_STEP_CP)
#define SAMPLING_GRAD_STEP
#endif
#if defined(SAMPLING_GRAD_SKIP) || defined(SAMPLING_GRAD_STEP)
#define SAMPLING_GRAD
#endif
#if defined(PVLEAF_GRAD_SKIP) || defined(PVLEAF_GRAD_STEP)
#define PVLEAF_GRAD
#endif

#if defined(LEARN_TD_LAMBDA_PROB) || defined(LEARN_TD_LAMBDA_CP)
#define LEARN_TD_LAMBDA
#endif
#if defined(LEARN_REGRESSION_LEAVES) || defined(LEARN_REGRESSION_ROOT)
#define LEARN_REGRESSION
#endif
#if defined(LEARN_TD_LAMBDA) || defined(LEARN_REGRESSION_LEAVES)
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
#ifdef LEARN_TD_LAMBDA_ROOT
	double tdr_rate = 0.01;
	double tdr_lambda = 0.9;
	double tdr_gamma = 0.95;

	LearnVec dw_tdr, tdr_e_vec;
	double tdr_Vt = 0.5;
	bool tdr_first = true;
#endif

#ifdef LEARN_REGRESSION_LEAVES
	double reg_leaves_rate = 0.01;
#endif
#ifdef LEARN_REGRESSION_ROOT
	double reg_root_rate = 0.01;
#endif
#ifdef LEARN_REGRESSION
	LearnVec dw_reg_win, dw_reg_lose;
#endif

#ifdef LEARN_PG
	double pg_rate = 0.01;
	LearnVec dw_pg;
#endif
#ifdef LEARN_PG_ROOT
	double pgroot_rate = 0.01;
	LearnVec dw_pgroot;
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