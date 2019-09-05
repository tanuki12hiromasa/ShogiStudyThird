#pragma once
#include "evaluator.h"

class SearchPlayer {
public:
	SearchPlayer(Kyokumen&& k,Feature&& f):kyokumen(k),feature(f){}
	void proceed(Move m) {
		feature.proceed(kyokumen,m);
		kyokumen.proceed(m);
	}

	Kyokumen kyokumen;
	Feature feature;
};