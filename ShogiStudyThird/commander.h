#pragma once
#include "agent.h"

class Commander {
public:
	static void execute();
private:
	Commander();
	~Commander();
	static void coutOption();
	void setOption(std::vector<std::string>& token);
	void paramInit();
	void gameInit();

	void go();
	void bestmove();
	void info();
	void sasu();

	SearchTree tree;
	std::vector<SearchAgent> agents;
	bool permitPonder;
	std::string filepath;

	std::vector<std::thread> agent_threads;
	std::thread go_thread;
	std::thread info_thread;
	std::atomic_bool go_alive;
	std::atomic_bool info_enable;
	std::atomic_bool info_alive;

	std::mutex coutmtx;
};