#pragma once
#include "evaluator.h"

class SearchPlayer {
public:
	SearchPlayer():kyokumen() { feature.set(kyokumen); }
	SearchPlayer(const Kyokumen& _kyokumen) :kyokumen(_kyokumen) { feature.set(kyokumen); }
	SearchPlayer(const Kyokumen& k, const Feature& f):kyokumen(k),feature(f){}

	Kyokumen kyokumen;
	Feature feature;

	void proceed(Move m) {
		feature.proceed(kyokumen,m);
		kyokumen.proceed(m);
	}
};