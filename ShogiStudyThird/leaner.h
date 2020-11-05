#pragma once
#include "agent.h"
#include "time_property.h"


class Learner {
public:
	static void execute(const std::string& enginename);
private:
	Learner();
	~Learner();
	static void coutOption();
	void setOption(const std::vector<std::string>& token);
	void paramInit();
	void gameInit();

	void selfplaylearn(int searchtime);
	void vslearn();
	void supervisedlearn();

	SearchTree tree;
	std::vector<std::unique_ptr<SearchAgent>> agents;
	std::unique_ptr<std::thread> deleteThread;
	int agentNum = 6;
	bool permitPonder;
	bool continuousTree = true;
	double Ts = 120;


	friend class ShogiTest;
};