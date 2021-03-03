#include "stdafx.h"
#include "kppt_evaluate.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace kppt {
	std::string kppt_evaluator::ifolderpath = "data/kppt_apery";
	std::string kppt_evaluator::ofolderpath = "data/learn/kppt";

	void kppt_evaluator::init() {
		kppt_feat::init(ifolderpath);
	}

	void kppt_evaluator::save() {
		//folderが無ければ作る
		kppt_feat::save(ofolderpath);
	}

	void kppt_evaluator::save(const std::string& path) {
		kppt_feat::save(path);
	}

	double kppt_evaluator::evaluate(const SearchPlayer& player) {
		return (double)player.feature.sum.sum(player.kyokumen.teban()) / FVScale;
	}

	void kppt_evaluator::genFirstEvalFile(const std::string& folderpath) {
		std::filesystem::create_directories(folderpath);
		auto* KPP = new KPPEvalElementType1[SquareNum];
		auto* KKP = new KKPEvalElementType1[SquareNum];
		memset(KPP, 0, sizeof(KPPEvalElementType1) * (size_t)SquareNum);
		memset(KKP, 0, sizeof(KKPEvalElementType1) * (size_t)SquareNum);
		{
			std::ofstream fs(folderpath + "/KPP.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KPP.bin) cannot make" << std::endl;
				return;
			}
			auto end = (char*)KPP + sizeof(KPPEvalElementType2);
			for (auto it = (char*)KPP; it < end; it += (1 << 30)) {
				size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
				fs.write(it, size);
			}
		}
		{
			std::ofstream fs(folderpath + "/KKP.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KKP.bin) cannot make" << std::endl;
				return;
			}
			auto end = (char*)KKP + sizeof(KKPEvalElementType2);
			for (auto it = (char*)KKP; it < end; it += (1 << 30)) {
				size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
				fs.write(it, size);
			}
		}
	}
}
