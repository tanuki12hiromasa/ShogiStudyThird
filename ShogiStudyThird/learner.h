#pragma once
#include "agent.h"
#include "learn_util.h"
#include "learn_method.h"

class Learner {
public:
	static void execute();

private:
	void init(const std::vector<std::string>& cmdtokens);
	void search(SearchTree& tree);
	void search(SearchTree& tree, const std::chrono::milliseconds time);
	static int getWinner(std::vector<std::string>& sfen);

	void learn_start_by_randompos(int batch, int itr);

	void learn_by_kifudata(const std::string& filepath);
	void learn_by_kifudata_latest(const std::string& dirpath);

	double T_search = 200;
	double T_selfplay = 120;
	std::chrono::milliseconds searchtime{ 1000 };
	int agentnum = 8;

	double child_pi_limit = 0.00005;
	double samplingrate = 0.1;

	double learning_rate_td = 0.1;
	double learning_rate_pp = 0.1;
	double learning_rate_bts = 0.1;
	double learning_rate_reg = 0.1;
	double learning_rate_pge = 0.1;
	double learning_rate_bts_sampling = 0.1;

	double td_gamma = 0.95;
	double td_lambda = 0.9;

	friend class ShogiTest;
};