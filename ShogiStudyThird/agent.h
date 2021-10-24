#pragma once
#include "tree.h"
#include "move_gen.h"
#include "temperature.h"
#include "random.h"
#include <random>
#include <functional>

//#define EXPAND_GRANDCHILDREN //探索での展開時に孫ノードまで展開するオプション

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
	SearchAgent(SearchTree& tree, const double Ts, const Random::xoshiro256p& seed);
	SearchAgent(SearchAgent&&)noexcept;
	~SearchAgent();
	SearchAgent() = delete;
	SearchAgent(const SearchAgent&) = delete;
	SearchAgent& operator=(const SearchAgent&) = delete;

	void loop();
private:
	void simulate(SearchNode* const root);
	size_t qsimulate(SearchNode* const root, SearchPlayer& player, const int hislength);
	bool checkRepetition(SearchNode* const node, const Kyokumen& kyokumen, const std::vector<SearchNode*>& history, const std::vector<std::pair<std::uint64_t, Bammen>>& k_history);
	bool checkRepetitiveCheck(const Kyokumen& k,const std::vector<SearchNode*>& searchhis, const SearchNode* const latestRepnode)const;
	bool deleteGarbage();

	SearchTree& tree;
	SearchPlayer player;
	std::thread th;
	const double Ts;
	std::atomic<state> status;
	std::atomic_bool searching;

	static std::atomic_bool search_enable;
	static std::atomic_uint64_t time_stamp;
	static std::atomic_uint old_threads_num;


	//値域 [0,1.0) のランダムな値
	Random::xoshiro256p random; //初期シードはコンストラクタで受け取る

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
	void joinPause();
	void noticeProceed();
	void deleteTree();
	void terminate();
private:
	SearchTree& tree;
	std::size_t agent_num = 8;
	std::size_t gc_num = 1;
	std::vector<std::unique_ptr<SearchAgent>> agents;

};