#include "stdafx.h"
#include "apery_learn.h"
#include <iostream>
#include <fstream>

namespace apery {
	void apery_paramVector::EvalClamp(std::int16_t absmax) {
		absmax = std::abs(absmax);
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < fe_end; p2++) {
					auto& vec = apery_feat::KPP[k][p1][p2];
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
					auto& vec = apery_feat::KPP[sk][gk][p];
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

	apery_paramVector::apery_paramVector() {
		KPP = new EvalVectorFloat[lkpptnum];
		KKP = new EvalVectorFloat[lkkptnum];
		reset();
	}

	apery_paramVector::apery_paramVector(apery_paramVector&& rhs) noexcept {
		KPP = rhs.KPP;
		KKP = rhs.KKP;
		rhs.KPP = nullptr;
		rhs.KKP = nullptr;
	}


	apery_paramVector& apery_paramVector::operator=(apery_paramVector&& rhs)noexcept {
		delete[] KPP;
		delete[] KKP;
		KPP = rhs.KPP;
		KKP = rhs.KKP;
		rhs.KPP = nullptr;
		rhs.KKP = nullptr;
		return *this;
	}

	apery_paramVector::~apery_paramVector() {
		delete[] KPP;
		delete[] KKP;
	}

	void apery_paramVector::reset() {
		EvalVectorFloat* const kpp = KPP;
		for (int i = 0; i < lkpptnum; i++) {
			kpp[i] = 0;
		}
		EvalVectorFloat* const kkp = KKP;
		for (int i = 0; i < lkkptnum; i++) {
			kkp[i] = 0;
		}
	}

	void apery_paramVector::addGrad(const float scalar, const SearchPlayer& player) {
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

				kpp[kpptToLkpptnum(skpos, k0, l0, 0)] += bammenscalar;
				kpp[kpptToLkpptnum(skpos, k0, l0, 1)] += tebanscalar;
				kpp[kpptToLkpptnum(invgkpos, k1, l1, 0)] -= bammenscalar;
				kpp[kpptToLkpptnum(invgkpos, k1, l1, 1)] += tebanscalar;
			}
			kkp[kkptToLkkptnum(skpos, gkpos, k0, 0)] += bammenscalar;
			kkp[kkptToLkkptnum(skpos, gkpos, k0, 1)] += tebanscalar;
		}
	}

	void apery_paramVector::clamp(float absmax) {
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
	}

	void apery_paramVector::updateEval() {
		clamp(1000);
		EvalClamp(30000);
		//KPPのテーブル形式の違いに注意する
		for (unsigned k = 0; k < SquareNum; k++) {
			for (unsigned p1 = 0; p1 < fe_end; p1++) {
				for (unsigned p2 = 0; p2 < p1; p2++) {
					int16_t val = KPP[kpptToLkpptnum(k, p1, p2, 0)];
					apery_feat::KPP[k][p1][p2][0] += val; apery_feat::KPP[k][p2][p1][0] += val;
					KPP[kpptToLkpptnum(k, p1, p2, 0)] -= val;
					val = KPP[kpptToLkpptnum(k, p1, p2, 1)];
					apery_feat::KPP[k][p1][p2][1] += val; apery_feat::KPP[k][p2][p1][1] += val;
					KPP[kpptToLkpptnum(k, p1, p2, 1)] -= val;
				}
			}
		}
		for (unsigned sk = 0; sk < SquareNum; sk++) {
			for (unsigned gk = 0; gk < SquareNum; gk++) {
				for (unsigned p = 0; p < fe_end; p++) {
					int16_t val = KKP[kkptToLkkptnum(sk, gk, p, 0)];
					apery_feat::KKP[sk][gk][p][0] += val;
					KKP[kkptToLkkptnum(sk, gk, p, 0)] -= val;
					val = KKP[kkptToLkkptnum(sk, gk, p, 1)];
					apery_feat::KKP[sk][gk][p][1] += val;
					KKP[kkptToLkkptnum(sk, gk, p, 1)] -= val;
				}
			}
		}
	}

	apery_paramVector& apery_paramVector::operator+=(const apery_paramVector& rhs) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] += rhs.KPP[i];
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] += rhs.KKP[i];
		}
		return *this;
	}
	apery_paramVector& apery_paramVector::operator+=(const fvpair& rhs) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] += rhs.f * rhs.v.KPP[i];
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] += rhs.f * rhs.v.KKP[i];
		}
		return *this;
	}
	apery_paramVector& apery_paramVector::operator*=(const double c) {
		for (size_t i = 0; i < lkpptnum; i++) {
			KPP[i] *= c;
		}
		for (size_t i = 0; i < lkkptnum; i++) {
			KKP[i] *= c;
		}
		return *this;
	}

	void apery_paramVector::save(const std::string& path) {
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
	}

	void apery_paramVector::load(const std::string& path) {
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
	}

	void apery_paramVector::showLearnVec(const double displaymin, int isKPP)const {
		using namespace std;
		if (isKPP) {
			cout << "show kpp" << endl;
			for (int i = 0; i < SquareNum; i++) {
				for (int j = 0; j < fe_end; j++) {
					for (int k = 0; k < j; k++) {
						if (j == k)continue;
						if (std::abs(KPP[kpptToLkpptnum(i, j, k, 0)]) > displaymin || std::abs(KPP[kpptToLkpptnum(i, j, k, 1)]) > displaymin) {
							cout << "kpp " << i << " " << j << " " << k << ": ";
							cout << KPP[kpptToLkpptnum(i, j, k, 0)] << " " << KPP[kpptToLkpptnum(i, j, k, 1)] << "\n";
						}
					}
				}
			}
		}
		else {
			cout << "show kkp" << endl;
			for (int i = 0; i < SquareNum; i++) {
				for (int j = 0; j < SquareNum; j++) {
					for (int k = 0; k < fe_end; k++) {
						if (std::abs(KKP[kkptToLkkptnum(i, j, k, 0)]) > displaymin || std::abs(KKP[kkptToLkkptnum(i, j, k, 1)]) > displaymin) {
							cout << "kkp " << i << " " << j << " " << k << ": ";
							cout << KKP[kkptToLkkptnum(i, j, k, 0)] << " " << KKP[kkptToLkkptnum(i, j, k, 1)] << "\n";
						}
					}
				}
			}
		}
	}

}