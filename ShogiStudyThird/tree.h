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
	
	void setTchoice(const std::vector<double>&);
	void setTchoice_q(const double Tcq) { T_choice_quiescence = Tcq; }
	void setTeval(const double Te) { T_eval = Te; }
	void setTdepth(const double Td) { T_depth = Td; }
	void setMassmaxInQSearch(const double mmqs) { MassMax_QS = mmqs; }


	void proceed(SearchNode* node);

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

private:
	void deleteTreeParallel(SearchNode* root);

	std::vector<SearchNode*> history;
	Kyokumen startKyokumen;
	SearchNode* rootNode;
	SearchNode* oldrootNode;
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

	std::vector<bool> thread_latestRootFlags;
	std::atomic_bool search_enable;
	std::mutex thmutex;
};
 
