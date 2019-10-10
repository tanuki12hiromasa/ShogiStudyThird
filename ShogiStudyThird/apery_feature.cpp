#include "stdafx.h"
#include "apery_feature.h"
#include <iostream>
#include <fstream>

namespace apery {
	void EvalList::set(const Kyokumen& kyokumen) {
		using namespace ::koma;
		int nlist = 0;
		material = 0;
#define Mochi(index0,index1,koma,teban) \
 for(int32_t i=kyokumen.getMochigomaNum(teban, koma); i>0; --i) {\
	 list0[nlist]=index0+i; list1[nlist]=index1+i;\
	 material+= teban ? PieceScore(koma) : -PieceScore(koma);\
	 ++nlist; \
 }
		Mochi(f_hand_pawn, e_hand_pawn, Mochigoma::Fu, true);
		Mochi(e_hand_pawn, f_hand_pawn, Mochigoma::Fu, false);
		Mochi(f_hand_lance, e_hand_lance, Mochigoma::Kyou, true);
		Mochi(e_hand_lance, f_hand_lance, Mochigoma::Kyou, false);
		Mochi(f_hand_knight, e_hand_knight, Mochigoma::Kei, true);
		Mochi(e_hand_knight, f_hand_knight, Mochigoma::Kei, false);
		Mochi(f_hand_silver, e_hand_silver, Mochigoma::Gin, true);
		Mochi(e_hand_silver, f_hand_silver, Mochigoma::Gin, false);
		Mochi(f_hand_gold, e_hand_gold, Mochigoma::Kin, true);
		Mochi(e_hand_gold, f_hand_gold, Mochigoma::Kin, false);
		Mochi(f_hand_bishop, e_hand_bishop, Mochigoma::Kaku, true);
		Mochi(e_hand_bishop, f_hand_bishop, Mochigoma::Kaku, false);
		Mochi(f_hand_rook, e_hand_rook, Mochigoma::Hi, true);
		Mochi(e_hand_rook, f_hand_rook, Mochigoma::Hi, false);
		assert(nlist <= 38);
#undef Mochi
		Bitboard bb = kyokumen.getAllBB();
		bb &= ~(kyokumen.getEachBB(Koma::s_Ou) | kyokumen.getEachBB(Koma::g_Ou));
		for (int i = bb.find_first(); i < bb.size(); i = bb.find_next(i)) {
			Koma koma = kyokumen.getKoma(i);
			list0[nlist] = komaToIndex(koma) + i;
			list1[nlist] = komaToIndex(sgInv(koma)) + inverse(i);
			material += PieceScore(koma);
			++nlist;
		}
		assert(nlist == 38);
	}

	KPPEvalElementType1* apery_feat::KPP;
	KKPEvalElementType1* apery_feat::KKP;
	bool apery_feat::allocated = false;
	void apery_feat::init() {
		if (!allocated) {
			allocated = true;
			KPP = new KPPEvalElementType1[SquareNum];
			KKP = new KKPEvalElementType1[SquareNum];
			memset(KPP, 0, sizeof(KPPEvalElementType1) * (size_t)SquareNum);
			memset(KKP, 0, sizeof(KKPEvalElementType1) * (size_t)SquareNum);
		}
		{
			std::ifstream fs("data/kppt_apery/KPP.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KPP.bin) cannot open" << std::endl;
				return;
			}
			auto end = (char*)KPP + sizeof(KPPEvalElementType2);
			for (auto it = (char*)KPP; it < end; it += (1 << 30)) {
				size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
				fs.read(it, size);
			}
		}
		{
			std::ifstream fs("data/kppt_apery/KKP.bin", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(KKP.bin) cannot open" << std::endl;
				return;
			}
			auto end = (char*)KKP + sizeof(KPPEvalElementType2);
			for (auto it = (char*)KKP; it < end; it += (1 << 30)) {
				size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
				fs.read(it, size);
			}
		}
	}

	EvalSum apery_feat::EvalFull(const Kyokumen& kyokumen, const EvalList& elist) {
		const unsigned skpos = kyokumen.sOuPos();
		const unsigned gkpos = kyokumen.gOuPos();
		const auto* ppskpp = KPP[skpos];
		const auto* ppgkpp = KPP[inverse(gkpos)];
		EvalSum sum;
		for (int i = 0; i < EvalList::EvalListSize; ++i) {
			const int k0 = elist.list0[i];
			const int k1 = elist.list1[i];
			const auto* pskpp = ppskpp[k0];
			const auto* pgkpp = ppgkpp[k1];
			for (int j = 0; j < i; j++) {
				const int l0 = elist.list0[j];
				const int l1 = elist.list1[j];
				sum.p[0] += pskpp[l0];
				sum.p[1] += pgkpp[l1];
			}
			sum.p[2] += KKP[skpos][gkpos][k0];
		}
		sum.p[2][0] += elist.material * FVScale;
		return sum;
	}

	void apery_feat::set(const Kyokumen& kyokumen) {
		idlist.set(kyokumen);
		sum = EvalFull(kyokumen, idlist);
	}

	//listとsumを差分更新する
	//listの更新前にsumから変更部分を引いて、list更新後にsumに新しい部分を足す
	void apery_feat::proceed(const Kyokumen& before, const Move& move) {
#define Replace(before,after,list) {int i=0;for(;i<EvalList::EvalListSize;i++){if(list[i]==before){list[i]=after;break;}}assert(i<38);}
		using namespace ::koma;
		if (!isInside(move.from)) { //駒台から
			Mochigoma m = MposToMochi(move.from);
			Koma k = MposToKoma(move.from);
			int mNum = before.getMochigomaNum(before.teban(), m);
			EvalIndex bIndex0 = mochiToIndex(m, before.teban()) + mNum;
			EvalIndex bIndex1 = mochiToIndex(m, !before.teban()) + mNum;
			EvalIndex aIndex0 = komaToIndex(k) + move.to;
			EvalIndex aIndex1 = komaToIndex(sgInv(k)) + inverse(move.to);

			const unsigned skpos = before.sOuPos();
			const unsigned gkpos = before.gOuPos();
			{
				const auto* pskpp = KPP[skpos][bIndex0];
				const auto* pgkpp = KPP[inverse(gkpos)][bIndex1];
				for (int i = 0; i < EvalList::EvalListSize; i++) {
					const auto index0 = idlist.list0[i];
					const auto index1 = idlist.list1[i];
					sum.p[0] -= pskpp[index0];
					sum.p[1] -= pgkpp[index1];
				}
				sum.p[2] -= KKP[skpos][gkpos][bIndex0];
			}
			Replace(bIndex0, aIndex0, idlist.list0);
			Replace(bIndex1, aIndex1, idlist.list1);
			{
				const auto* pskpp = KPP[skpos][aIndex0];
				const auto* pgkpp = KPP[inverse(gkpos)][aIndex1];
				for (int i = 0; i < EvalList::EvalListSize; i++) {
					const auto index0 = idlist.list0[i];
					const auto index1 = idlist.list1[i];
					sum.p[0] += pskpp[index0];
					sum.p[1] += pgkpp[index1];
				}
				sum.p[2] += KKP[skpos][gkpos][aIndex0];
			}
		}
		else {
			Koma k = before.getKoma(move.from);
			Koma cap = before.getKoma(move.to);
			if (k == Koma::s_Ou) {//先手玉が動いた場合
				const unsigned gkpos = before.gOuPos();
				if (cap != Koma::None) {
					//駒を取った場合 非手番側の駒が取られて手番側の駒になる
					//gKPPを差分計算 sKPP,KKPはあとで全計算する
					Mochigoma m_cap = KomaToMochi(cap);
					int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					EvalIndex bcIndex0 = komaToIndex(cap) + move.to;
					EvalIndex bcIndex1 = komaToIndex(sgInv(cap)) + inverse(move.to);
					EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					EvalIndex acIndex1 = mochiToIndex(m_cap, !before.teban()) + mNum;
					{
						const auto* pgkpp = KPP[inverse(gkpos)][bcIndex1];
						for (auto index : idlist.list1) {
							sum.p[1] -= pgkpp[index];
						}
					}
					Replace(bcIndex0, acIndex0, idlist.list0);
					Replace(bcIndex1, acIndex1, idlist.list1);
					idlist.material -= PieceScore(cap);
					idlist.material += PieceScore(m_cap, before.teban());
					{
						const auto* pgkpp = KPP[inverse(gkpos)][acIndex1];
						for (auto index : idlist.list1) {
							sum.p[1] += pgkpp[index];
						}
					}
				}
				//sKPP,KKP全体を再計算
				//玉はリストにないのでリストは更新されない
				sum.p[0][0] = 0; sum.p[0][1] = 0;
				sum.p[2][0] = idlist.material * FVScale; sum.p[2][1] = 0;
				const unsigned skpos = move.to;
				const auto* ppskpp = KPP[skpos];
				const auto* pkkp = KKP[skpos][gkpos];
				for (int i = 0; i < EvalList::EvalListSize; i++) {
					const auto index0_i = idlist.list0[i];
					for (int j = 0; j < i; j++) {
						const auto index0_j = idlist.list0[j];
						sum.p[0] += ppskpp[index0_i][index0_j];
					}
					sum.p[2] += pkkp[index0_i];
				}
			}
			else if (k == Koma::g_Ou) {//後手玉が動いた場合
				const unsigned skpos = before.sOuPos();
				if (cap != Koma::None) {
					//駒を取った場合 非手番側の駒が取られて手番側の駒になる
					//sKPPを差分計算
					Mochigoma m_cap = KomaToMochi(cap);
					int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
					EvalIndex bcIndex0 = komaToIndex(cap) + move.to;
					EvalIndex bcIndex1 = komaToIndex(sgInv(cap)) + inverse(move.to);
					EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
					EvalIndex acIndex1 = mochiToIndex(m_cap, !before.teban()) + mNum;
					{
						const auto* pskpp = KPP[skpos][bcIndex0];
						for (int i = 0; i < EvalList::EvalListSize; i++) {
							const int index0 = idlist.list0[i];
							sum.p[0] -= pskpp[index0];
						}
					}
					Replace(bcIndex0, acIndex0, idlist.list0);
					Replace(bcIndex1, acIndex1, idlist.list1);
					idlist.material -= PieceScore(cap);
					idlist.material += PieceScore(m_cap, before.teban());
					{
						const auto* pskpp = KPP[skpos][acIndex0];
						for (int i = 0; i < EvalList::EvalListSize; i++) {
							const int index0 = idlist.list0[i];
							sum.p[0] += pskpp[index0];
						}
					}
				}
				//gKPP,KKP全体を再計算
				sum.p[1][0] = 0; sum.p[1][1] = 0;
				sum.p[2][0] = idlist.material * FVScale; sum.p[2][1] = 0;
				const unsigned gkpos = move.to;
				const auto* ppgkpp = KPP[inverse(gkpos)];
				const auto* pkkpi = KKP[skpos][gkpos];
				for (int i = 0; i < EvalList::EvalListSize; i++) {
					const auto index1_i = idlist.list1[i];
					for (int j = 0; j < i; j++) {
						const auto index1_j = idlist.list1[j];
						sum.p[1] += ppgkpp[index1_i][index1_j];
					}
					const auto index0_i = idlist.list0[i];
					sum.p[2] += pkkpi[index0_i];
				}
			}
			else if (cap != Koma::None) {//駒をとっていた場合
				Koma k = before.getKoma(move.from);
				EvalIndex bIndex0 = komaToIndex(k) + move.from;
				EvalIndex bIndex1 = komaToIndex(sgInv(k)) + inverse(move.from);
				if (move.promote) {
					int material_diff = -PieceScore(k);
					k = promote(k);
					material_diff += PieceScore(k);
					idlist.material += material_diff;
					sum.p[2][0] += material_diff * FVScale;
				}
				EvalIndex aIndex0 = komaToIndex(k) + move.to;
				EvalIndex aIndex1 = komaToIndex(sgInv(k)) + inverse(move.to);

				Mochigoma m_cap = KomaToMochi(cap);
				int mNum = before.getMochigomaNum(before.teban(), m_cap) + 1;
				EvalIndex bcIndex0 = komaToIndex(cap) + move.to;
				EvalIndex bcIndex1 = komaToIndex(sgInv(cap)) + inverse(move.to);
				EvalIndex acIndex0 = mochiToIndex(m_cap, before.teban()) + mNum;
				EvalIndex acIndex1 = mochiToIndex(m_cap, !before.teban()) + mNum;

				const unsigned skpos = before.sOuPos();
				const unsigned gkpos = before.gOuPos();
				{
					const auto* pskpp = KPP[skpos][bIndex0];
					const auto* pgkpp = KPP[inverse(gkpos)][bIndex1];
					const auto* cpskpp = KPP[skpos][bcIndex0];
					const auto* cpgkpp = KPP[inverse(gkpos)][bcIndex1];
					for (int i = 0; i < EvalList::EvalListSize; i++) {
						const auto index0 = idlist.list0[i];
						const auto index1 = idlist.list1[i];
						sum.p[0] -= pskpp[index0];
						sum.p[1] -= pgkpp[index1];
						sum.p[0] -= cpskpp[index0];
						sum.p[1] -= cpgkpp[index1];
					}
					sum.p[2] -= KKP[skpos][gkpos][bIndex0];
					sum.p[2] -= KKP[skpos][gkpos][bcIndex0];
				}
				Replace(bIndex0, aIndex0, idlist.list0);
				Replace(bIndex1, aIndex1, idlist.list1);
				Replace(bcIndex0, acIndex0, idlist.list0);
				Replace(bcIndex1, acIndex1, idlist.list1);
				{
					int material_diff = PieceScore(m_cap, before.teban()) - PieceScore(cap);
					idlist.material += material_diff;
					sum.p[2][0] += material_diff * FVScale;
				}
				{
					const auto* pskpp = KPP[skpos][aIndex0];
					const auto* pgkpp = KPP[inverse(gkpos)][aIndex1];
					const auto* cpskpp = KPP[skpos][acIndex0];
					const auto* cpgkpp = KPP[inverse(gkpos)][acIndex1];
					for (int i = 0; i < EvalList::EvalListSize; i++) {
						const auto index0 = idlist.list0[i];
						const auto index1 = idlist.list1[i];
						sum.p[0] += pskpp[index0];
						sum.p[1] += pgkpp[index1];
						sum.p[0] += cpskpp[index0];
						sum.p[1] += cpgkpp[index1];
					}
					sum.p[2] += KKP[skpos][gkpos][aIndex0];
					sum.p[2] += KKP[skpos][gkpos][acIndex0];
				}
			}
			else {//玉以外の駒が何も取らずに動いたとき
				Koma k = before.getKoma(move.from);
				EvalIndex bIndex0 = komaToIndex(k) + move.from;
				EvalIndex bIndex1 = komaToIndex(sgInv(k)) + inverse(move.from);
				if (move.promote) {
					int material_diff = -PieceScore(k);
					k = promote(k);
					material_diff += PieceScore(k);
					idlist.material += material_diff;
					sum.p[2][0] += material_diff * FVScale;
				}
				EvalIndex aIndex0 = komaToIndex(k) + move.to;
				EvalIndex aIndex1 = komaToIndex(sgInv(k)) + inverse(move.to);

				const unsigned skpos = before.sOuPos();
				const unsigned gkpos = before.gOuPos();
				{
					const auto* pskpp = KPP[skpos][bIndex0];
					const auto* pgkpp = KPP[inverse(gkpos)][bIndex1];
					for (int i = 0; i < EvalList::EvalListSize; i++) {
						const auto index0 = idlist.list0[i];
						const auto index1 = idlist.list1[i];
						sum.p[0] -= pskpp[index0];
						sum.p[1] -= pgkpp[index1];
					}
					sum.p[2] -= KKP[skpos][gkpos][bIndex0];
				}
				Replace(bIndex0, aIndex0, idlist.list0);
				Replace(bIndex1, aIndex1, idlist.list1);
				{
					const auto* pskpp = KPP[skpos][aIndex0];
					const auto* pgkpp = KPP[inverse(gkpos)][aIndex1];
					for (int i = 0; i < EvalList::EvalListSize; i++) {
						const auto index0 = idlist.list0[i];
						const auto index1 = idlist.list1[i];
						sum.p[0] += pskpp[index0];
						sum.p[1] += pgkpp[index1];
					}
					sum.p[2] += KKP[skpos][gkpos][aIndex0];
				}
			}
		}
#undef Replace
	}
}