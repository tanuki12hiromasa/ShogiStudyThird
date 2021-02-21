#include "stdafx.h"
#include "apery_evaluate.h"

namespace apery {
	void apery_evaluator::init() {
		apery_feat::init();
	}


	double apery_evaluator::evaluate(const SearchPlayer& player) {
		return (double)player.feature.sum.sum(player.kyokumen.teban()) / FVScale;
	}
}
