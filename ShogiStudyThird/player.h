#pragma once
#include "feature.h"

class SearchPlayer {
public:
	Kyokumen kyokumen;
	Feature feature;

	SearchPlayer():kyokumen(),feature() {}
	SearchPlayer(const Kyokumen& _kyokumen) :kyokumen(_kyokumen) { feature.set(kyokumen); }
	SearchPlayer(const Kyokumen& k, const Feature& f):kyokumen(k),feature(f){}

	koma::Koma proceed(Move m) {
		feature.proceed(kyokumen,m);
		return kyokumen.proceed(m);
	}
	void back(Move m, koma::Koma captured, FeaureCache cache) {
		kyokumen.back(m, captured);
		feature.back(kyokumen, m, cache);
	}
};