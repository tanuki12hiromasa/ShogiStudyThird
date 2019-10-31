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
public:
	static void setMateScore(const double score) { mateScore = score; }
	static void setMateScoreBound(const double bound) { mateScoreBound = bound; }
	static void setMateOneScore(const double score) { mateOneScore = score; }
	static void setRepScore(const double score) { repetitionScore = score; }
	static double getMateScoreBound() { return mateScoreBound; }
public:
	SearchNode(const Move& move);
	SearchNode(const SearchNode&) = delete;
	SearchNode(SearchNode&&) = delete;

	size_t deleteTree();//子孫ノードをすべて消す 自身は消さない
	SearchNode* addChild(const Move& move);

	void setEvaluation(const double evaluation) { eval = evaluation; }
	void setMass(const double m) { mass = m; }

	void setMateVariation(const double childmin);
	void setMate();
	void setUchiFuMate();
	void setDeclare();
	void setRepetition(const double m);
	void setRepetitiveCheck(const double m);

	double getEvaluation()const { return eval.load(); }
	bool isNotExpanded()const { return state == State::N; }
	bool isLimitedExpanded()const { return state == State::QE || state == State::QT; }
	bool isQSTerminal()const { return state != State::N && state != State::QE; }
	bool isLeaf()const { return state == State::N || state == State::QE || state == State::QT; }
	bool isTerminal()const { return state == State::T; }

	std::vector<SearchNode*> children;
	Move move;
	std::atomic<State> state;
	std::atomic<double> eval;
	std::atomic<double> mass;
};