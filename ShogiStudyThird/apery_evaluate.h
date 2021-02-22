#pragma once
#include "apery_feature.h"
#include "player.h"

namespace apery {
	using SearchPlayer = Player<apery_feat, EvalSum>;
	class apery_evaluator {
	public:
		static void init();

		static double evaluate(const SearchPlayer& player);
	};
}