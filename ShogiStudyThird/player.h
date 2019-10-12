#pragma once
#include "feature.h"

class SearchPlayer {
public:
	Kyokumen kyokumen;
	Feature feature;

	SearchPlayer():kyokumen() { feature.set(kyokumen); }
	SearchPlayer(const Kyokumen& _kyokumen) :kyokumen(_kyokumen) { feature.set(kyokumen); }
	SearchPlayer(const Kyokumen& k, const Feature& f):kyokumen(k),feature(f){}

	void proceed(Move m) {
		feature.proceed(kyokumen,m);
		kyokumen.proceed(m);
	}
};