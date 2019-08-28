#include "stdafx.h"
#include "evaluated_nodes.h"

void EvaluatedNodes_diff::add(Move move) {
	nodes.push_back(parentNode->addChild(move));
}

void EvaluatedNodes_full::add(Move move) {
	Kyokumen nextKyokumen(kyokumen);
	nextKyokumen.proceed(move);
	nodes.emplace_back(evaluatedNode(parentNode->addChild(move), std::move(nextKyokumen)));
}