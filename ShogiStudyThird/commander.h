#pragma once
#include "agent.h"

class Commander {
public:
	static void execute();
private:
	Commander();
	void paramInit(std::string filepath);
	void gameInit();

	void go();
	void ponder();

	SearchTree tree;
	std::vector<SearchAgent> agents;
	bool permitPonder = false;

	std::vector<std::thread> agent_threads;
	std::thread go_thread;
	std::atomic_bool go_enable;
	std::atomic_bool ponder_enable;

	std::mutex coutmtx;
};