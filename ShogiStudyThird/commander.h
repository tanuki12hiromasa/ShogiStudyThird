﻿#pragma once
#include "agent.h"
#include "time_property.h"
#include "joseki.h"

class Commander {
public:
	static void execute(const std::string& enginename);
protected:
	Commander();
	~Commander();
	static void coutOption();
	void setOption(const std::vector<std::string>& token);
	void paramInit();
	void gameInit();

	void go(const std::vector<std::string>& tokens);
	std::pair<std::chrono::milliseconds, std::chrono::milliseconds> decide_timelimit(const TimeProperty time)const;
	void info();
	void chakushu(SearchNode* const bestmove);
	void position(const std::vector<std::string>& tokens);

	SearchTree tree;
	AgentPool agents{ tree };
	std::thread deleteThread;
	bool permitPonder;
	bool interimInfo = true;
	bool continuousTree = true;
	int resign_border = 3;
	std::chrono::milliseconds time_quickbm_lower{ 4000 };
	std::chrono::milliseconds time_standard_upper{ 20000 };
	std::chrono::milliseconds time_overhead {150};
	int estimate_movesnum = 120;

	std::thread go_thread;
	std::thread info_thread;
	std::atomic_bool go_alive;
	std::atomic_bool info_enable;
	std::atomic_bool info_alive;
	uint64_t info_prev_evcount;
	std::chrono::time_point<std::chrono::system_clock> info_prevtime;

	std::mutex coutmtx;
	std::mutex treemtx;

	friend class ShogiTest;

	Joseki joseki;
};