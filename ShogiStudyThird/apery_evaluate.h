#pragma once
#include "apery_feature.h"
#include "evaluated_nodes.h"
#include "player.h"

namespace apery {
	class apery_evaluator {
	public:
		static void init();
		static void evaluate(EvaluatedNodes_full& en);
		static void evaluate(EvaluatedNodes_diff& en, const apery_feat& pfeat);
		static void evaluate(std::vector<SearchNode*> nodes, const SearchPlayer& player);
	};
}