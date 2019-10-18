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
		NotExpanded, LimitExpanded, LimitExpandedTerminal, ExpandedInQuiescence, EQTerminal, 
		Expanded, MateVariation, CheckMate, RepetitiveCheck, Repetition,
		NE = NotExpanded, LE = LimitExpanded, LT = LimitExpandedTerminal, EQ = ExpandedInQuiescence, ET = EQTerminal, 
		EX = Expanded, MV = MateVariation, CM = CheckMate, RC = RepetitiveCheck, RP = Repetition
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

	void deleteTree();//子孫ノードをすべて消す 自身は消さない
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
	bool isNotExpanded()const { return state == State::NE; }
	bool isLimitedExpanded()const { return state == State::LE || state == State::LT; }
	bool isQSTerminal()const { return !isNotExpanded() && state != State::LE && state != State::EQ; }
	bool isLeaf()const { return state == State::NE || state == State::LE || state == State::LT || state == State::EQ || state==State::ET; }
	bool isRepetition()const { return state == State::RP || state == State::RC; }

	std::vector<SearchNode*> children;
	Move move;
	std::atomic<State> state;
	std::atomic<double> eval;
	std::atomic<double> mass;
};