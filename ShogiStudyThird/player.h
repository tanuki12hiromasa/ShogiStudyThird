#pragma once
#include "evaluator.h"

class SearchPlayer {
public:
	Kyokumen kyokumen;
	Feature feature;

	SearchPlayer():kyokumen() { feature.set(kyokumen); }
	SearchPlayer(const Kyokumen& _kyokumen) :kyokumen(_kyokumen) { feature.set(kyokumen); }
	SearchPlayer(Kyokumen&& k,Feature&& f):kyokumen(k),feature(f){}
	void proceed(Move m) {
		feature.proceed(kyokumen,m);
		kyokumen.proceed(m);
	}
};