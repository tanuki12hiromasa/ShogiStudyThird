#pragma once
#include "nnue_feature.h"
#include "../player.h"

namespace Eval::NNUE {
	using SearchPlayer = Player<NNUE_feat, Accumulator>;
	class NNUE_evaluator {
	public:
		static void init();
		static double evalute(const SearchPlayer& player);
	};
}