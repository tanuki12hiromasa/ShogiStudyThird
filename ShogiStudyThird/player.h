#pragma once
#include "kyokumen.h"

template <typename Feat, typename Cache>
class Player {
public:
	Kyokumen kyokumen;
	Feat feature;

	Player():kyokumen(),feature() {}
	Player(const Kyokumen& _kyokumen) :kyokumen(_kyokumen) { feature.set(kyokumen); }
	Player(const Kyokumen& k, const Feat& f):kyokumen(k),feature(f){}

	koma::Koma proceed(Move m) {
		feature.proceed(kyokumen,m);
		return kyokumen.proceed(m);
	}
	std::pair<koma::Koma, Cache> proceedC(Move m) {
		auto cache = feature.getCache();
		feature.proceed(kyokumen, m);
		auto cap = kyokumen.proceed(m);
		return std::make_pair(cap, cache);
	}
	void recede(Move m, koma::Koma captured, Cache cache) {
		const auto moved = kyokumen.recede(m, captured);
		feature.recede(kyokumen, moved, captured, m, cache);
	}
	void recede(Move m, std::pair<koma::Koma, Cache> cache) {
		const auto moved = kyokumen.recede(m, cache.first);
		feature.recede(kyokumen, moved, cache.first, m, cache.second);
	}
	bool operator==(const Player& rhs) const {
		return kyokumen == rhs.kyokumen && feature == rhs.feature;
	}
};