#pragma once
#include "learn_util.h"
#include "agent.h"

class Learner {
public:
	static void execute();

private:
	void init(const std::vector<std::string>& cmdtokens);
	void search(SearchTree& tree);
	void search(SearchTree& tree, const std::chrono::milliseconds time);
	static int getWinner(std::vector<std::string>& sfen);
	LearnVec simple_bootstrap(const Kyokumen startpos, const std::vector<Move>& kifu, const int winner, const bool learnteban);
	LearnVec sampling_bootstrap(const Kyokumen startpos, const std::vector<Move>& kifu, const int winner, const bool learnteban);

	void selfplay_rootstrap(LearnVec& dw);
	void selfplay_bootstrap(LearnVec& dw);
	void selfplay_child_bootstrap(LearnVec& dw);
	void selfplay_sampling_regression(LearnVec& dw);
	void selfplay_sampling_pge(LearnVec& dw);
	void selfplay_sampling_td(LearnVec& dw);
	void selfplay_sampling_bts(int samplingnum, double droprate = 0);

	double T_search = 200;
	double T_selfplay = 120;
	std::chrono::milliseconds searchtime{ 10000 };
	int agentnum = 8;

	double child_pi_limit = 0.00005;
	double samplingrate = 0.1;

	double learning_rate_td = 0.1;
	double learning_rate_pp = 0.1;
	double learning_rate_bts = 0.1;
	double learning_rate_reg = 0.1;
	double learning_rate_pge = 0.5;
	double learning_rate_bts_sampling = 0.1;

	double td_gamma = 0.95;
	double td_lambda = 0.99;

	friend class ShogiTest;
};