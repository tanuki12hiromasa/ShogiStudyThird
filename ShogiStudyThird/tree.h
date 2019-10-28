#pragma once
#include "node.h"
#include "kyokumen.h"
#include "evaluator.h"
#include "player.h"
#include <unordered_map>

class SearchTree {
public:
	SearchTree();
	void set(const Kyokumen& startpos,const std::vector<Move>& moves);
	void set(const std::vector<std::string>& usitokens);

	void setNodeMaxsize(const size_t s) { nodesMaxCount = s; }
	void setTchoice(const std::vector<double>&);
	void setTchoice_q(const double Tcq) { T_choice_quiescence = Tcq; }
	void setTeval(const double Te) { T_eval = Te; }
	void setTdepth(const double Td) { T_depth = Td; }
	void setMassmaxInQSearch(const double mmqs) { MassMax_QS = mmqs; }

	void permitSearch() { search_enable = true; }
	void prohibitSearch() { search_enable = false; }
	SearchNode* getBestMove()const;//最もevalの高いrootのchildを返す
	std::vector<SearchNode*> getPV()const;//rootからのpvの連なりを返す
	void proceed(SearchNode* node);
	void deleteBranchParallel(SearchNode* base, SearchNode* saved, uint8_t oldhisnum);//baseのsaved以下以外の探索木を子ノードを残して消去する

	const uint64_t getNodeCount() const { return nodecount; }
	const std::vector<SearchNode*>& getHistory()const { return history; }
	const SearchPlayer& getRootPlayer()const { return rootPlayer; }
	double getTchoice();
	double getTcQ()const { return T_choice_quiescence; }
	double getTeval()const { return T_eval; }
	double getTdepth()const { return T_depth; }
	double getMQS()const { return MassMax_QS; }

	bool resisterLeafNode(SearchNode* const node);
	void excludeLeafNode(SearchNode* const node);

	SearchNode* getRoot(unsigned threadNo, size_t increaseNodes);
	SearchNode* getRoot() const { return history.back(); }

	void foutTree()const;
private:
	void deleteTreeParallel(SearchNode* root,uint8_t oldhisnum);//rootを含め子孫を全消去する

	std::vector<SearchNode*> history;
	Kyokumen startKyokumen;
	SearchPlayer rootPlayer;
	std::atomic_uint64_t nodecount;
	std::uint64_t nodesMaxCount;

	std::array<double, 64> T_choice;
	std::atomic_uint64_t T_c_count;
	double T_choice_quiescence;
	double T_eval;
	double T_depth;
	double MassMax_QS;

	std::unordered_map<SearchNode*, unsigned> nmap;
	std::mutex lnmutex;

	std::vector<std::uint8_t> lastRefRootByThread;
	std::atomic_bool search_enable;
	std::mutex thmutex;

	friend class Commander;
	friend class ShogiTest;
};
 
