#pragma once
#include "node.h"
#include "kyokumen.h"
#include "move.h"


//評価値で差分計算をするならEVAL_DIFF_ONLYを定義する
#define EVAL_DIFF_ONLY

//評価関数に子ノードをまとめて渡すためのクラス
class EvaluatedNodes_diff {
public:
	SearchNode* const parentNode;//親
	const Kyokumen kyokumen;
	std::vector<SearchNode*> nodes;
	bool evaluated = false;//評価が終了したかどうか

	EvaluatedNodes_diff(SearchNode* parentNode, const Kyokumen& parentKyokumen)
		: parentNode(parentNode), kyokumen(parentKyokumen)
	{}
	void add(Move move);
};

class EvaluatedNodes_full {
public:
	struct evaluatedNode {
		evaluatedNode(SearchNode* const node, Kyokumen&& nkyokumen)
			:node(node), nextKyokumen(std::move(nkyokumen))
		{}
		SearchNode* const node;
		const Kyokumen nextKyokumen;
	};
	SearchNode* const parentNode;//親
	const Kyokumen kyokumen;
	std::vector<evaluatedNode> nodes;
	bool evaluated = false;//評価が終了したかどうか

	EvaluatedNodes_full(SearchNode* parentNode, const Kyokumen& parentKyokumen)
		: parentNode(parentNode), kyokumen(parentKyokumen)
	{}
	void add(Move move);
};

#ifdef EVAL_DIFF_ONLY
using EvaluatedNodes = EvaluatedNodes_diff;
#else
using EvaluatedNodes = EvaluatedNodes_full;
#endif // EVAL_DIFF_ONLY
