#pragma once
#include "learn_util.h"

class LearnMethod {
public:
	LearnMethod(LearnVec& vec) :dw(vec) {}
	virtual void update(SearchNode* const root, const SearchPlayer& player) = 0;
	virtual void fin(SearchNode* const root, const SearchPlayer& player, GameResult result) {}
	virtual unsigned long long getSamplingPosNum() { return samplingPosNum; }
protected:
	LearnVec& dw;
	unsigned long long samplingPosNum = 0;
};

class RootStrap : public LearnMethod {
public:
	RootStrap(LearnVec& dw, const double& learn_rate)
		:rate(learn_rate), LearnMethod(dw) {}

	void update(SearchNode* const root, const SearchPlayer& player);

private:
	const double rate;
};

class SamplingBTS :public LearnMethod {
public:
	SamplingBTS(LearnVec& dw, const double& learn_rate, const size_t& sampling_num, const double& T)
		:rate(learn_rate), sampling_num(sampling_num), T(T), LearnMethod(dw) {}
	void update(SearchNode* const root, const SearchPlayer& player);
	void fin(SearchNode* const root, const SearchPlayer& player, GameResult result)override;
private:
	const double rate;
	const size_t sampling_num;
	const double T;
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
};