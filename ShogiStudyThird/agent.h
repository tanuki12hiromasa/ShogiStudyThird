#pragma once
#include "tree.h"
#include "move_gen.h"
#include <random>

class SearchAgent {
public:
	static void setLeaveQSNode(bool b) { leave_QsearchNode = b; }
	static void setUseOriginalKyokumenEval(bool b) { use_original_kyokumen_eval = b; }
	static void setQSrelativeDepth(bool b) { QS_relativeDepth = b; }
	static void setDrawMoveNum(int n) { drawmovenum = n; }
private:
	static bool leave_QsearchNode;
	static bool use_original_kyokumen_eval;
	static bool QS_relativeDepth;
	static int drawmovenum;

	enum class state {
		search, gc, terminate
	};
public:
	SearchAgent(SearchTree& tree, const double Ts, int seed, std::atomic_bool& enable,
		std::atomic_uint& old_threads_num, std::atomic_uint64_t& stamp);
	SearchAgent(SearchAgent&&)noexcept;
	~SearchAgent();
	SearchAgent() = delete;
	SearchAgent(const SearchAgent&) = delete;
	SearchAgent& operator=(const SearchAgent&) = delete;

	void loop();
private:
	void simulate(SearchNode* const root);
	size_t qsimulate(SearchNode* const root, SearchPlayer& player, const int hislength);
	bool checkRepetitiveCheck(const Kyokumen& k,const std::vector<SearchNode*>& searchhis, const SearchNode* const latestRepnode)const;
	bool deleteGarbage();

	SearchTree& tree;
	SearchPlayer player;
	std::atomic_bool& enable;
	std::thread th;
	const double Ts;
	std::atomic<state> status;

	std::atomic_uint64_t& stamp;
	std::atomic_uint& old_threads_num;

	//値域 [0,1.0) のランダムな値
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine; //初期シードはコンストラクタで受け取る

	friend class ShogiTest;
	friend class AgentPool;
};

class AgentPool {
public:
	AgentPool(SearchTree& tree);
	~AgentPool() { terminate(); }
	void setAgentNum(std::size_t num) { agent_num = num; }
	void setup();
	void startSearch();
	void pauseSearch();
	void noticeProceed();
	void deleteTree();
	void terminate();
private:
	SearchTree& tree;
	std::size_t agent_num = 8;
	std::size_t gc_num = 1;
	std::vector<std::unique_ptr<SearchAgent>> agents;
	std::atomic_uint64_t time_stamp;
	std::atomic_uint old_thread_num;
	std::atomic_bool search_enable;
	std::vector<double> TsDistribution = { 120 };
};