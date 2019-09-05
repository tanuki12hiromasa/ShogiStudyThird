#include "stdafx.h"
#include "apery_evaluate.h"

namespace apery {
	void apery_evaluator::init() {
		apery_feat::init();
	}

	void apery_evaluator::evaluate(EvaluatedNodes_full& en) {
		//一局面ずつ全計算している
		for (auto& node : en.nodes) {
			apery_feat feat(node.nextKyokumen);
			node.node->setEvaluation((double)feat.sum.sum(node.nextKyokumen.teban()) / FVScale);
		}
#if 0
		//親局面から差分計算 正直中途半端である パフォーマンス比較には使えるか?
		apery_feat pfeat(en.kyokumen);
		for (auto& node : en.nodes) {
			apery_feat cfeat(pfeat);
			cfeat.proceed(en.kyokumen, node.node->move);
			node.node->setEvaluation((double)cfeat.sum.sum(node.nextKyokumen.teban()) / FVScale);
		}
#endif
	}

	void apery_evaluator::evaluate(EvaluatedNodes_diff& en, const apery_feat& pfeat) {
		//rootノードから展開ノードまで進めてきたfeatで差分計算する
		for (auto& node : en.nodes) {
			apery_feat cfeat(pfeat);
			cfeat.proceed(en.kyokumen, node->move);
			node->setEvaluation((double)cfeat.sum.sum(!en.kyokumen.teban()) / FVScale);
		}
	}

	void apery_evaluator::evaluate(std::vector<SearchNode*> nodes, const SearchPlayer& player) {
		//rootノードから展開ノードまで進めてきたfeatで差分計算する
		for (auto& node : nodes) {
			apery_feat cfeat(player.feature);
			cfeat.proceed(player.kyokumen, node->move);
			node->setEvaluation((double)cfeat.sum.sum(!player.kyokumen.teban()) / FVScale);
		}
	}
}
