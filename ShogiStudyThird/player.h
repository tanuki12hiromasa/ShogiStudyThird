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
	void recede(Move m, koma::Koma captured, FeaureCache cache) {
		const auto moved = kyokumen.recede(m, captured);
		feature.recede(kyokumen, moved, captured, m, cache);
	}
	bool operator==(const SearchPlayer& rhs) const {
		return kyokumen == rhs.kyokumen && feature == rhs.feature;
	}
};