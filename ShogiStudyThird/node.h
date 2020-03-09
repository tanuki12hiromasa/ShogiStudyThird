#pragma once
#include "move.h"
#include <shared_mutex>
#include <atomic>
#include <vector>

using s_mutex = std::shared_mutex;
using s_lock = std::shared_lock<std::shared_mutex>;
using p_lock = std::lock_guard<std::shared_mutex>;

class SearchNode {
	friend class LeafGuard;
	friend class SearchTree;
public:
	enum class State : std::int8_t {
		NotExpanded, inExpanding, Expanded, Terminal,
		N = NotExpanded, iE=inExpanding, E = Expanded, T = Terminal
	};
private:
	static double mateMass;
	static double mateScore;
	static double mateScoreBound;
	static double mateOneScore;
	static double repetitionScore;
	static double Ts_c;
	static int Ts_FunctionCode;//探索指標の分散を期待値で重みづけするかどうかのフラグ
	static double T_eval;
	static double T_depth;
	static int QS_depth;
	static int Es_FunctionCode;
	static double Es_c;
	static int PV_FuncCode;
	static double PV_c;
public:
	static void setMateScore(const double score) { mateScore = score; }
	static void setMateScoreBound(const double bound) { mateScoreBound = bound; }
	static void setMateOneScore(const double score) { mateOneScore = score; }
	static void setRepScore(const double score) { repetitionScore = score; }
	static double getMateScoreBound() { return mateScoreBound; }
	static double getMateScore() { return mateScore; }
	static void setTsFuncParam(const double Ts) { Ts_c = Ts; }
	static void setTsFuncCode(int c) { Ts_FunctionCode = c; }
	static void setTeval(const double Te) { T_eval = Te; }
	static void setTdepth(const double Td) { T_depth = Td; }
	static void setQSearchDepth(const double mmqs) { QS_depth = mmqs; }
	static double getTeval() { return T_eval; }
	static double getTdepth() { return T_depth; }
	static double getQSdepth() { return QS_depth; }
	static void setEsFuncCode(const int code) { Es_FunctionCode = code; }
	static void setEsFuncParam(const double c) { Es_c = c; }
	static void setPVFuncCode(const int code) { PV_FuncCode = code; }
	static void setPVConst(const double b) { PV_c = b; }
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
	void setOriginEval(const double evaluation) { origin_eval = evaluation; }

	double getEvaluation()const { return eval.load(); }
	bool isLeaf()const { const auto s = status.load(); return s == State::N || s == State::iE; }
	bool isTerminal()const { return status == State::T; }
	bool isSearchable()const { const auto s = status.load(); return s == State::N || s == State::E; }
	double getTs(const double baseT)const;
	double getEs()const;
	SearchNode* getBestChild()const;
private:
	double getTcMcVariance()const;
	double getTcMcVarianceExpection()const;
public:
	std::vector<SearchNode*> children;
	Move move;
private:
	std::atomic<State> status;
	std::int32_t origin_eval;
public:
	std::atomic<double> eval;
	std::atomic<double> mass;

};