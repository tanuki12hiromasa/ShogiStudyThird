#pragma once
#include "move.h"
#include <shared_mutex>
#include <atomic>
#include <vector>

using s_mutex = std::shared_mutex;
using s_lock = std::shared_lock<std::shared_mutex>;
using p_lock = std::lock_guard<std::shared_mutex>;

class SearchNode {
public:
	enum class State : std::int8_t {
		NotExpanded, QuiescenceExpanded, QuiescenceTerminal, Expanded, Terminal,
		N = NotExpanded, QE = QuiescenceExpanded, QT = QuiescenceTerminal, E = Expanded, T = Terminal
	};
private:
	static double mateMass;
	static double mateScore;
	static double mateScoreBound;
	static double mateOneScore;
	static double repetitionScore;
	static double Tc_const;
	static double Tc_mp;
	static double Tc_mc;
	static bool Tc_mc_expectable_variance;//探索指標の分散を期待値で重みづけするかどうかのフラグ
	static double T_eval;
	static double T_depth;
	static double MassMax_QS;
	static int Ec_FunctionCode;
	static double Ec_c;
public:
	static void setMateScore(const double score) { mateScore = score; }
	static void setMateScoreBound(const double bound) { mateScoreBound = bound; }
	static void setMateOneScore(const double score) { mateOneScore = score; }
	static void setRepScore(const double score) { repetitionScore = score; }
	static double getMateScoreBound() { return mateScoreBound; }
	static void setTcConst(const double Tc) { Tc_const = Tc; }
	static void setTcmp(const double Tc) { Tc_mp = Tc; }
	static void setTcmc(const double Tc) { Tc_mc = Tc; }
	static void setTcmc_expectable_flag(bool b) { Tc_mc_expectable_variance = b; }
	static void setTeval(const double Te) { T_eval = Te; }
	static void setTdepth(const double Td) { T_depth = Td; }
	static void setMassmaxInQSearch(const double mmqs) { MassMax_QS = mmqs; }
	static double getTeval() { return T_eval; }
	static double getTdepth() { return T_depth; }
	static double getMQS() { return MassMax_QS; }
	static void setEcFuncCode(const int code) { Ec_FunctionCode = code; }
	static void setEcC(const double c) { Ec_c = c; }
public:
	SearchNode(const Move& move);
	SearchNode(const SearchNode&) = delete;
	SearchNode(SearchNode&&) = delete;

	size_t deleteTree();//子孫ノードをすべて消す 自身は消さない
	SearchNode* addChild(const Move& move);
	SearchNode* addCopyChild(const SearchNode* const origin);

	void setEvaluation(const double evaluation) { eval = evaluation; }
	void setMass(const double m) { mass = m; }

	void setMateVariation(const double childmin);
	void setMate();
	void setDeclare();
	void setRepetition(const bool teban);
	void setRepetitiveCheck();
	void setExpandedAll() { expanded = true; }
	void setOriginEval(const double evaluation) { origin_eval = evaluation; eval = evaluation; }
	void addVisitCount() { visit_count++; }

	double getEvaluation()const { return eval.load(); }
	bool isNotExpanded()const { return status == State::N; }
	bool isLimitedExpanded()const { return status == State::QE || status == State::QT; }
	bool isQSTerminal()const { return status != State::N && status != State::QE; }
	bool isLeaf()const { return status == State::N || status == State::QE || status == State::QT; }
	bool isTerminal()const { return status == State::T; }
	bool isExpandedAll() { return expanded; }
	double getT_c()const;
	size_t getVisitCount()const { return visit_count; }
	double getE_c(const size_t& visitnum_p, const double& mass_p)const;
private:
	double getTcMcVariance()const;
public:
	std::vector<SearchNode*> children;
	Move move;
	std::atomic<State> status;
private:
	bool expanded;
	std::int32_t origin_eval;
	size_t visit_count;
public:
	std::atomic<double> eval;
	std::atomic<double> mass;

};