#include "stdafx.h"
#include "kppt_evaluate.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace kppt {

	std::string kppt_evaluator::ifolderpath = "data/kppt";
	std::string kppt_evaluator::ofolderpath = "data/kppt";

	void kppt_evaluator::init() {
		kppt_feat::init(ifolderpath);
	}
	void kppt_evaluator::save() {
		std::filesystem::create_directories(ofolderpath);
		kppt_feat::save(ofolderpath);
	}
	void kppt_evaluator::save(const std::string& path) {
		std::filesystem::create_directories(path);
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
#ifdef KPPT_DYNAMIC_PIECE_SCORE
		{
			const std::array<PieceScoreType, static_cast<size_t>(koma::Koma::KomaNum)> PieceScoreArr = { 0 };
			std::ofstream fs(folderpath + "/Piece.bin", std::ios::binary);
			if (fs) {
				for (auto& p : PieceScoreArr) {
					fs.write((char*)&p, sizeof(p));
				}
			}
			else {
				std::cerr << "error:file(" << folderpath << "/Piece.bin) cannot open" << std::endl;
			}
		}
#endif
	}

	void kppt_evaluator::print(int iskpp) {
		using namespace std;
		if (iskpp) {
			cout << "show kpp" << endl;
			for (int i = 0; i < kppt::SquareNum; i++) {
				for (int j = 0; j < kppt::fe_end; j++) {
					for (int k = 0; k < j; k++) {
						if (j == k)continue;
						if (KPP[i][j][k][0] != 0 || KPP[i][j][k][1] != 0)
							cout << i << "," << j << "," << k << ": " << KPP[i][j][k][0] << " " << KPP[i][j][k][1] << "\n";
					}
				}
			}
		}
		else {
			cout << "show kkp" << endl;
			for (int i = 0; i < kppt::SquareNum; i++) {
				for (int j = 0; j < kppt::SquareNum; j++) {
					for (int k = 0; k < kppt::fe_end; k++) {
						if (KKP[i][j][k][0] != 0 || KKP[i][j][k][1] != 0)
							cout << i << "," << j << "," << k << ": " << KKP[i][j][k][0] << " " << KKP[i][j][k][1] << "\n";
					}
				}
			}
		}
	}

}
