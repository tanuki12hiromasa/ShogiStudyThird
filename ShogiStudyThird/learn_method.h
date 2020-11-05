#pragma once
#include "learn_util.h"

class LearnMethod {
	virtual void learn_turn(LearnVec& dw, SearchNode* const root, const SearchPlayer& rootplayer);
	virtual void learn_gameend(LearnVec& dw, SearchNode* const root, const SearchPlayer& rootplayer, bool senteWin);
};

