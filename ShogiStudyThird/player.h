#pragma once
#include "evaluator.h"

class SearchPlayer {
public:
	SearchPlayer(Kyokumen&& k,Feature&& f):kyokumen(k),feature(f){}
	void proceed(Move m) {
#ifdef EVAL_DIFF_ONLY
		feature.proceed(kyokumen,m);
#endif
		kyokumen.proceed(m);
	}

	Kyokumen kyokumen;
#ifdef EVAL_DIFF_ONLY
	Feature feature;
#endif
};