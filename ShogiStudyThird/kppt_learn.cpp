#include "kppt_learn.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace kppt {
	void kppt_paramVector::EvalClamp(std::int16_t absmax) {
		absmax = std::abs(absmax);
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < fe_end; p2++) {
					auto& vec = kppt::KPP[k][p1][p2];
					if (vec[0] > absmax) {
						vec[0] = absmax;
					}
					else if (vec[0] < -absmax) {
						vec[0] = -absmax;
					}
					if (vec[1] > absmax) {
						vec[1] = absmax;
					}
					else if (vec[1] < -absmax) {
						vec[1] = -absmax;
					}
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					auto& vec = kppt::KPP[sk][gk][p];
					if (vec[0] > absmax) {
						vec[0] = absmax;
					}
					else if (vec[0] < -absmax) {
						vec[0] = -absmax;
					}
					if (vec[1] > absmax) {
						vec[1] = absmax;
					}
					else if (vec[1] < -absmax) {
						vec[1] = -absmax;
					}
				}
			}
		}
	}

	kppt_paramVector::kppt_paramVector() {
		KPP = new EvalVectorFloat[lkpptnum];
		KKP = new EvalVectorFloat[lkkptnum];
		reset();
	}

	kppt_paramVector::kppt_paramVector(kppt_paramVector&& rhs) noexcept :
		PieceScoreArr(std::move(rhs.PieceScoreArr))
	{
		KPP = rhs.KPP;
		KKP = rhs.KKP;
		rhs.KPP = nullptr;
		rhs.KKP = nullptr;
	}

	kppt_paramVector& kppt_paramVector::operator=(kppt_paramVector&& rhs)noexcept {
		delete[] KPP;
		delete[] KKP;
		KPP = rhs.KPP;
		KKP = rhs.KKP;
		rhs.KPP = nullptr;
		rhs.KKP = nullptr;
		return *this;
	}

	kppt_paramVector::~kppt_paramVector() {
		delete[] KPP;
		delete[] KKP;
	}

	void kppt_paramVector::reset() {
		EvalVectorFloat* const kpp = KPP;
		for (int i = 0; i < lkpptnum; i++) {
			kpp[i] = 0;
		}
		EvalVectorFloat* const kkp = KKP;
		for (int i = 0; i < lkkptnum; i++) {
			kkp[i] = 0;
		}
		for (auto& p : PieceScoreArr) {
			p = 0;
		}
	}

	inline void kpp_addGrad(EvalVectorFloat* const kpp, const int kpos, const int k, const int l, const float bg, const float tg) {
		kpp[kpptToLkpptnum(kpos, k, l, 0)] += bg;
		kpp[kpptToLkpptnum(kpos, k, l, 1)] += tg;
		kpp[kpptToLkpptnum(koma::mirrorX(kpos), mirror((EvalIndex)k), mirror((EvalIndex)l), 0)] += bg;
		kpp[kpptToLkpptnum(koma::mirrorX(kpos), mirror((EvalIndex)k), mirror((EvalIndex)l), 1)] += tg;
	}
	inline void kkp_addGrad(EvalVectorFloat* const kkp, const int skpos, const int gkpos, const int k, const float bg, const float tg) {
		kkp[kkptToLkkptnum(skpos, gkpos, k, 0)] += bg;
		kkp[kkptToLkkptnum(skpos, gkpos, k, 1)] += tg;
		kkp[kkptToLkkptnum(koma::mirrorX(skpos), koma::mirrorX(gkpos), mirror((EvalIndex)k), 0)] += bg;
		kkp[kkptToLkkptnum(koma::mirrorX(skpos), koma::mirrorX(gkpos), mirror((EvalIndex)k), 1)] += tg;
	}
	void kppt::kppt_paramVector::piece_addGrad(const float scalar, const Kyokumen& kyokumen) {
		{ //歩
			const int fu = (int)kyokumen.getEachBB(koma::Koma::s_Fu).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Fu).popcount();
			PieceScoreArr[0] += scalar * fu;
		}
		{ //香
			const int kyou = (int)kyokumen.getEachBB(koma::Koma::s_Kyou).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Kyou).popcount();
			PieceScoreArr[1] += scalar * kyou;
		}
		{ //桂
			const int kei = (int)kyokumen.getEachBB(koma::Koma::s_Kei).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Kei).popcount();
			PieceScoreArr[2] += scalar * kei;
		}
		{ //銀
			const int gin = (int)kyokumen.getEachBB(koma::Koma::s_Gin).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Gin).popcount();
			PieceScoreArr[3] += scalar * gin;
		}
		{ //角
			const int kaku = (int)kyokumen.getEachBB(koma::Koma::s_Kaku).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Kaku).popcount();
			PieceScoreArr[4] += scalar * kaku;
		}
		{ //飛
			const int hi = (int)kyokumen.getEachBB(koma::Koma::s_Hi).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Hi).popcount();
			PieceScoreArr[5] += scalar * hi;
		}
		{ //金
			const int kin = (int)kyokumen.getEachBB(koma::Koma::s_Kin).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_Kin).popcount();
			PieceScoreArr[6] += scalar * kin;
		}
		{ //と金
			const int nfu = (int)kyokumen.getEachBB(koma::Koma::s_nFu).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nFu).popcount();
			PieceScoreArr[7] += scalar * nfu;
		}
		{ //成香
			const int nkyou = (int)kyokumen.getEachBB(koma::Koma::s_nKyou).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nKyou).popcount();
			PieceScoreArr[8] += scalar * nkyou;
		}
		{ //成桂
			const int nkei = (int)kyokumen.getEachBB(koma::Koma::s_nKei).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nKei).popcount();
			PieceScoreArr[9] += scalar * nkei;
		}
		{ //成銀
			const int ngin = (int)kyokumen.getEachBB(koma::Koma::s_nGin).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nGin).popcount();
			PieceScoreArr[10] += scalar * ngin;
		}
		{ //馬
			const int uma = (int)kyokumen.getEachBB(koma::Koma::s_nKaku).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nKaku).popcount();
			PieceScoreArr[11] += scalar * uma;
		}
		{ //龍
			const int ryu = (int)kyokumen.getEachBB(koma::Koma::s_nHi).popcount() - (int)kyokumen.getEachBB(koma::Koma::g_nHi).popcount();
			PieceScoreArr[12] += scalar * ryu;
		}
	}

	void kppt_paramVector::addGrad(const float scalar, const SearchPlayer& player) {
		if (std::abs(scalar) < 0.00000001f) return;
		EvalVectorFloat* const kpp = KPP;
		EvalVectorFloat* const kkp = KKP;
		const unsigned skpos = player.kyokumen.sOuPos();
		const unsigned gkpos = player.kyokumen.gOuPos();
		const unsigned invgkpos = inverse(gkpos);
		const unsigned invskpos = inverse(skpos);
		const float bammenscalar = (player.kyokumen.teban()) ? scalar : -scalar;
		const float tebanscalar = scalar;
		for (unsigned i = 0; i < EvalList::EvalListSize; ++i) {
			const int k0 = player.feature.idlist.list0[i];
			const int k1 = player.feature.idlist.list1[i];
			for (unsigned j = 0; j < i; j++) {
				const int l0 = player.feature.idlist.list0[j];
				const int l1 = player.feature.idlist.list1[j];

				kpp_addGrad(kpp, skpos, k0, l0, bammenscalar, tebanscalar);
				kpp_addGrad(kpp, invgkpos, k1, l1, -bammenscalar, tebanscalar);
			}
			kkp_addGrad(kkp, skpos, gkpos, k0, bammenscalar, tebanscalar);
			kkp_addGrad(kkp, invgkpos, invskpos, k1, -bammenscalar, -tebanscalar);
		}
		piece_addGrad(scalar, player.kyokumen);
	}

	void kppt_paramVector::clamp(float absmax) {
		absmax = std::abs(absmax);
		for (size_t i = 0; i < lkpptnum; i++) {
			if (KPP[i] > absmax) {
				KPP[i] = absmax;
			}
			else if (KPP[i] < -absmax) {
				KPP[i] = -absmax;
			}
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			if (KKP[i] > absmax) {
				KKP[i] = absmax;
			}
			else if (KKP[i] < -absmax) {
				KKP[i] = -absmax;
			}
		}
		for (size_t i = 0; i < lpiecenum; i++) {
			if (PieceScoreArr[i] > absmax) {
				PieceScoreArr[i] = absmax;
			}
			else if (PieceScoreArr[i] < -absmax) {
				PieceScoreArr[i] = -absmax;
			}
		}
	}

	void kppt_paramVector::updateEval() {
		//KPPのテーブル形式の違いに注意する
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < p1; p2++) {
					EvalElementTypeh val = KPP[kpptToLkpptnum(k, p1, p2, 0)];
					kppt::KPP[k][p1][p2][0] += val; kppt::KPP[k][p2][p1][0] += val;
					KPP[kpptToLkpptnum(k, p1, p2, 0)] -= val;
					val = KPP[kpptToLkpptnum(k, p1, p2, 1)];
					kppt::KPP[k][p1][p2][1] += val; kppt::KPP[k][p2][p1][1] += val;
					KPP[kpptToLkpptnum(k, p1, p2, 1)] -= val;
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					EvalElementTypeh val = KKP[kkptToLkkptnum(sk, gk, p, 0)];
					kppt::KKP[sk][gk][p][0] += val;
					KKP[kkptToLkkptnum(sk, gk, p, 0)] -= val;
					val = KKP[kkptToLkkptnum(sk, gk, p, 1)];
					kppt::KKP[sk][gk][p][1] += val;
					KKP[kkptToLkkptnum(sk, gk, p, 1)] -= val;
				}
			}
		}
#ifdef KPPT_DYNAMIC_PIECE_SCORE
		for (size_t i = 0; i < lpiecenum_plain; i++) {
			const PieceScoreType val = PieceScoreArr[i];
			kppt::PieceScoreArr[i] += val;
			kppt::PieceScoreArr[i + 10] -= val;
			PieceScoreArr[i] -= val;
		}
		//王の分を学習時に数えていないので、その分飛ばす
		for (size_t i = lpiecenum_plain; i < lpiecenum; i++) {
			const PieceScoreType val = PieceScoreArr[i];
			kppt::PieceScoreArr[i + 1] += val;
			kppt::PieceScoreArr[i + 11] -= val;
			PieceScoreArr[i] -= val;
		}
#endif
	}

	kppt_paramVector& kppt_paramVector::operator+=(const kppt_paramVector& rhs) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] += rhs.KPP[i];
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] += rhs.KKP[i];
		}
		return *this;
	}
	kppt_paramVector& kppt_paramVector::operator+=(const fvpair& rhs) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] += rhs.f * rhs.v.KPP[i];
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] += rhs.f * rhs.v.KKP[i];
		}
		return *this;
	}
	kppt_paramVector& kppt_paramVector::operator*=(const double c) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] *= c;
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] *= c;
		}
		return *this;
	}

	void kppt_paramVector::save(const std::string& path) {
		std::ofstream fs(path, std::ios::binary);
		if (!fs) {
			std::cerr << "error:file canot generate" << std::endl;
			return;
		}
		for (auto it = (char*)KPP, end = (char*)KPP + sizeof(KPPEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.write(it, size);
		}
		for (auto it = (char*)KKP, end = (char*)KKP + sizeof(KKPEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.write(it, size);
		}
#ifdef KPPT_DYNAMIC_PIECE_SCORE
		{
			std::ofstream fs_p(path + "p", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(piece) canot generate" << std::endl;
				return;
			}
			for (const auto& p : PieceScoreArr) {
				fs_p.write((char*)&p, sizeof(p));
			}
		}
#endif
	}

	void kppt_paramVector::load(const std::string& path) {
		std::ifstream fs(path, std::ios::binary);
		if (!fs) {
			std::cerr << "error:file canot open" << std::endl;
			return;
		}
		for (auto it = (char*)KPP, end = (char*)KPP + sizeof(KPPEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.read(it, size);
		}
		for (auto it = (char*)KKP, end = (char*)KKP + sizeof(KKPEvalVectorFloat); it < end; it += (1 << 30)) {
			size_t size = (it + (1 << 30) < end ? (1 << 30) : end - it);
			fs.read(it, size);
		}
#ifdef KPPT_DYNAMIC_PIECE_SCORE
		{
			std::ifstream fs_p(path + "p", std::ios::binary);
			if (!fs) {
				std::cerr << "error:file(piece) canot open" << std::endl;
				return;
			}
			for (const auto& p : PieceScoreArr) {
				fs_p.read((char*)&p, sizeof(p));
			}
		}
#endif
	}

	void kppt_paramVector::print(const double displaymin, int isKPP)const {
		using namespace std;
		if (isKPP) {
			cout << "show kpp" << endl;
			for (int i = 0; i < kppt::SquareNum; i++) {
				for (int j = 0; j < kppt::fe_end; j++) {
					for (int k = 0; k < j; k++) {
						if (j == k)continue;
						if (std::abs(KPP[kppt::kpptToLkpptnum(i, j, k, 0)]) > displaymin || std::abs(KPP[kppt::kpptToLkpptnum(i, j, k, 1)]) > displaymin) {
							cout << "kpp " << i << " " << j << " " << k << ": ";
							cout << KPP[kppt::kpptToLkpptnum(i, j, k, 0)] << " " << KPP[kppt::kpptToLkpptnum(i, j, k, 1)] << "\n";
						}
					}
				}
			}
		}
		else {
			cout << "show kkp" << endl;
			for (int i = 0; i < kppt::SquareNum; i++) {
				for (int j = 0; j < kppt::SquareNum; j++) {
					for (int k = 0; k < kppt::fe_end; k++) {
						if (std::abs(KKP[kppt::kkptToLkkptnum(i, j, k, 0)]) > displaymin || std::abs(KKP[kppt::kkptToLkkptnum(i, j, k, 1)]) > displaymin) {
							cout << "kkp " << i << " " << j << " " << k << ": ";
							cout << KKP[kppt::kkptToLkkptnum(i, j, k, 0)] << " " << KKP[kppt::kkptToLkkptnum(i, j, k, 1)] << "\n";
						}
					}
				}
			}
		}
#ifdef KPPT_DYNAMIC_PIECE_SCORE
		cout << "show Piece" << endl;
		for (int i = 0; i < lpiecenum; i++) {
			cout << i << ":" << PieceScoreArr[i] << " ";
		}
		cout << endl;
#endif
	}

	bool kppt_paramVector::operator==(const kppt_paramVector& rhs)const {
		for (size_t i = 0; i < lkpptnum; i++) {
			if (KPP[i] != rhs.KPP[i]) return false;
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			if (KKP[i] != rhs.KKP[i]) return false;
		}
#ifdef KPPT_DYNAMIC_PIECE_SCORE
		if (PieceScoreArr != rhs.PieceScoreArr) {
			return false;
		}
#endif
		return true;
	}

	void Adam::updateEval(kppt_paramVector& dw) {
		t++;
		const auto b1t = std::pow(b1, t);
		const auto b2t = std::pow(b2, t);
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < p1; p2++) {
					{
						const auto idx = kpptToLkpptnum(k, p1, p2, 0);
						const auto val = dw.KPP[idx];
						mt.KPP[idx] = mt.KPP[idx] * b1 + (1 - b1) * val;
						vt.KPP[idx] = vt.KPP[idx] * b2 + (1 - b2) * val * val;
						const double dwt = -alpha * (mt.KPP[idx] / (1 - b1t)) / (std::sqrt(vt.KPP[idx] / (1 - b2t)) + epsilon);
						kppt::KPP[k][p1][p2][0] += dwt; kppt::KPP[k][p2][p1][0] += dwt;
						/*if (std::abs(dwt) > 0) {
							std::cout << "t:"<< t << " val:" << val;
							std::cout << " mt,vt:" << mt.KPP[idx] << "," << vt.KPP[idx];
							std::cout << " dwt:" << dwt;
							std::cout << "\n";
						}*/
						dw.KPP[idx] = 0;
					}
					{
						const auto idx = kpptToLkpptnum(k, p1, p2, 1);
						const double val = dw.KPP[idx];
						mt.KPP[idx] = mt.KPP[idx] * b1 + (1 - b1) * val;
						vt.KPP[idx] = vt.KPP[idx] * b2 + (1 - b2) * val * val;
						const double dwt = -alpha * (mt.KPP[idx] / (1 - b1t)) / (std::sqrt(vt.KPP[idx] / (1 - b2t)) + epsilon);
						kppt::KPP[k][p1][p2][1] += dwt; kppt::KPP[k][p2][p1][1] += dwt;
						dw.KPP[idx] = 0;
					}
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					{
						const auto idx = kkptToLkkptnum(sk, gk, p, 0);
						const auto val = dw.KKP[idx];
						mt.KKP[idx] = mt.KKP[idx] * b1 + (1 - b1) * val;
						vt.KKP[idx] = vt.KKP[idx] * b2 + (1 - b2) * val * val;
						const double dwt = -alpha * (mt.KKP[idx] / (1 - b1t)) / (std::sqrt(vt.KKP[idx] / (1 - b2t)) + epsilon);
						kppt::KKP[sk][gk][p][0] += dwt;
						/*if (std::abs(dwt) > 0) {
							std::cout << "t:" << t << " val:" << val;
							std::cout << " mt,vt:" << mt.KKP[idx] << "," << vt.KKP[idx];
							std::cout << " dwt:" << std::fixed << std::setprecision(6) << dwt;
							std::cout << "\n";
						}*/
						dw.KKP[idx] = 0;
					}
					{
						const auto idx = kkptToLkkptnum(sk, gk, p, 1);
						const auto val = dw.KKP[idx];
						mt.KKP[idx] = mt.KKP[idx] * b1 + (1 - b1) * val;
						vt.KKP[idx] = vt.KKP[idx] * b2 + (1 - b2) * val * val;
						const double dwt = -alpha * (mt.KKP[idx] / (1 - b1t)) / (std::sqrt(vt.KKP[idx] / (1 - b2t)) + epsilon);
						kppt::KKP[sk][gk][p][1] += dwt;
						dw.KKP[idx] = 0;
					}
				}
			}
		}
#ifdef KPPT_DYNAMIC_PIECE_SCORE
		for (size_t i = 0; i < lpiecenum; i++) {
			const auto val = dw.PieceScoreArr[i];
			mt.PieceScoreArr[i] = mt.PieceScoreArr[i] * b1 + (1 - b1) * val;
			vt.PieceScoreArr[i] = vt.PieceScoreArr[i] * b2 + (1 - b2) * val * val;
			const double dwt = -alpha * (mt.PieceScoreArr[i] / (1 - b1t)) / (std::sqrt(vt.PieceScoreArr[i] / (1 - b2t)) + epsilon);
			kppt::PieceScoreArr[i] += dwt;
			dw.PieceScoreArr[i] = 0;
		}
#endif
	}

	void Adam::save(const std::string& path) {
		std::filesystem::create_directories(path);
		{
			std::ofstream fs(path + "/adam.bin");
			fs << t << std::endl;
		}
		mt.save(path + "/mt.bin");
		vt.save(path + "/vt.bin");
	}
	void Adam::load(const std::string& path) {
		{
			std::ifstream fs(path + "/adam.bin");
			fs >> t;
		}
		mt.load(path + "/mt.bin");
		vt.load(path + "/vt.bin");
	}
}