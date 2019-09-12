#pragma once
#include "node.h"
#include "kyokumen.h"
#include "evaluator.h"
#include "player.h"

class SearchTree {
public:
	SearchTree();
	void clear();
	
	void setT(double T_e, double T_d, std::vector<double> T_c);
	void setTchoice(const std::vector<double>&);
	void setTchoice_q(const double Tcq) { T_choice_quiescence = Tcq; }
	void setTeval(const double Te) { T_eval = Te; }
	void setTdepth(const double Td) { T_depth = Td; }

	void proceed(SearchNode* node);
	void addNodenum(int n) { nodenum += n; }

	SearchNode* getRoot();
	const std::vector<SearchNode*>& getHistory()const { return history; }
	const SearchPlayer& getRootPlayer()const { return rootPlayer; }
	double getTchoice()const;
	double getTcQ()const { return T_choice_quiescence; }
	double getTeval()const { return T_eval; }
	double getTdepth()const { return T_depth; }
	double getMQS()const { return MassMax_QS; }

	std::mutex& getMutex(const SearchNode* const node);
	void restoreMutex(const SearchNode* const node);

private:
	SearchNode* rootNode;
	SearchNode* oldrootNode;
	SearchPlayer rootPlayer;
	std::vector<SearchNode*> history;
	Kyokumen startKyokumen;
	std::atomic_uint64_t nodenum;

	std::array<double, 64> T_choice;
	std::atomic_uint T_c_count;
	double T_choice_quiescence;
	double T_eval;
	double T_depth;
	double MassMax_QS;

};