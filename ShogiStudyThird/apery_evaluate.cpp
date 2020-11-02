#include "stdafx.h"
#include "apery_evaluate.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace apery {

	std::string apery_evaluator::ifolderpath = "data/kppt";
	std::string apery_evaluator::ofolderpath = "data/learn/kppt";

	void apery_evaluator::init(const std::string& path) {
		apery_feat::init(path);
	}

	void apery_evaluator::save(const std::string& path) {
		apery_feat::save(path);
	}

	void apery_evaluator::evaluate(EvaluatedNodes_full& en) {
		//一局面ずつ全計算している
		for (auto& node : en.nodes) {
			apery_feat feat(node.nextKyokumen);
			node.node->setOriginEval((double)feat.sum.sum(node.nextKyokumen.teban()) / FVScale);
		}
#if 0
		//親局面から差分計算 正直中途半端である パフォーマンス比較には使えるか?
		apery_feat pfeat(en.kyokumen);
		for (auto& node : en.nodes) {
			apery_feat cfeat(pfeat);
			cfeat.proceed(en.kyokumen, node.node->move);
			node.node->setOriginEval((double)cfeat.sum.sum(node.nextKyokumen.teban()) / FVScale);
		}
#endif
	}

	void apery_evaluator::evaluate(EvaluatedNodes_diff& en, const apery_feat& pfeat) {
		//rootノードから展開ノードまで進めてきたfeatで差分計算する
		for (auto& node : en.nodes) {
			apery_feat cfeat(pfeat);
			cfeat.proceed(en.kyokumen, node->move);
			node->setOriginEval((double)cfeat.sum.sum(!en.kyokumen.teban()) / FVScale);
		}
	}

	void apery_evaluator::evaluate(std::vector<SearchNode*> nodes, const SearchPlayer& player) {
		//rootノードから展開ノードまで進めてきたfeatで差分計算する
		for (auto& node : nodes) {
			apery_feat cfeat(player.feature);
			cfeat.proceed(player.kyokumen, node->move);
			node->setOriginEval((double)cfeat.sum.sum(!player.kyokumen.teban()) / FVScale);
		}
	}

	void apery_evaluator::evaluate(SearchNode* node, const SearchPlayer& player) {
		node->setOriginEval((double)player.feature.sum.sum(player.kyokumen.teban()) / FVScale);
	}

	double apery_evaluator::evaluate(const SearchPlayer& player) {
		return (double)player.feature.sum.sum(player.kyokumen.teban()) / FVScale;
	}

	void apery_evaluator::genInitalPram(const std::string& folderpath) {
		std::filesystem::create_directories(folderpath);
		{
			auto* KPP = new KPPEvalElementType2;
			memset(KPP, 0, sizeof(KPPEvalElementType2));
			std::ofstream fs(folderpath + "/KPP.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KPP.bin) cannot open" << std::endl;
				return;
			}
			fs.write(reinterpret_cast<char*>(KPP), sizeof(KPPEvalElementType2));
			delete[] KPP;
		}
		{
			auto* KKP = new KKPEvalElementType2;
			memset(KKP, 0, sizeof(KKPEvalElementType2));
			std::ofstream fs(folderpath + "/KKP.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KKP.bin) cannot open" << std::endl;
				return;
			}
			fs.write(reinterpret_cast<char*>(KKP), sizeof(KKPEvalElementType2));
			delete[] KKP;
		}
		std::cout << "Parameters have been generated to " << folderpath << std::endl;
	}
}
