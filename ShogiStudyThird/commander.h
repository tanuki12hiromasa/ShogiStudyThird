#pragma once
#include "agent.h"
#include "time_property.h"

class Commander {
public:
	static void execute();
private:
	Commander();
	~Commander();
	static void coutOption();
	void setOption(const std::vector<std::string>& token);
	void paramInit();
	void gameInit();
	void setTsDistribution();

	void startAgent();
	void stopAgent();
	void go(const std::vector<std::string>& tokens);
	void info();
	void chakushu();
	void position(const std::vector<std::string>& tokens);
	void releaseAgentAndBranch(SearchNode* const prevRoot, std::vector<SearchNode*>&& newNodes);
	void releaseAgentAndTree(SearchNode* const root);

	void yomikomi();

	SearchTree tree;
	std::vector<std::unique_ptr<SearchAgent>> agents;
	std::unique_ptr<std::thread> deleteThread;
	int agentNum = 6;
	bool permitPonder;
	bool continuousTree = true;
	double Ts_min = 40;
	double Ts_max = 200;
	int TsDistFuncNum = 0;
	std::vector<double> TsDistribution;

	std::thread go_thread;
	std::thread info_thread;
	std::atomic_bool go_alive;
	std::atomic_bool info_enable;
	std::atomic_bool info_alive;

	std::mutex coutmtx;
	std::mutex treemtx;

	bool yomikomi_on = false;	//定跡を有効化
	std::string yomikomi_file_name = "treejoseki";	//定跡木の名前(.txtは不要)
	static void sortChildren(SearchNode* node);
	
	friend class ShogiTest;
};