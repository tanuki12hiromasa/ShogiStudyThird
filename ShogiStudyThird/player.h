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
	std::pair<koma::Koma, FeaureCache> proceedC(Move m) {
		auto cache = feature.getCache();
		feature.proceed(kyokumen, m);
		auto cap = kyokumen.proceed(m);
		return std::make_pair(cap, cache);
	}
	void recede(Move m, koma::Koma captured, FeaureCache cache) {
		const auto moved = kyokumen.recede(m, captured);
		feature.recede(kyokumen, moved, captured, m, cache);
	}
	void recede(Move m, std::pair<koma::Koma, FeaureCache> cache) {
		const auto moved = kyokumen.recede(m, cache.first);
		feature.recede(kyokumen, moved, cache.first, m, cache.second);
	}
	bool operator==(const SearchPlayer& rhs) const {
		return kyokumen == rhs.kyokumen && feature == rhs.feature;
	}
};