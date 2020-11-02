#pragma once
#include "apery_evaluate.h"
#include <cassert>
#include <functional>

namespace apery {
	constexpr size_t lkpptnum = SquareNum * fe_end * (fe_end - 1); //kpptテーブルのppの被りを消した三角テーブルを一次元にしている セル数はSQnum*(fe*(fe-1)/2)*2
	constexpr size_t lkkptnum = SquareNum * SquareNum * fe_end * 2;//こちらは先後の区別があるため単純に一次元にしている
	using EvalVectorFloat = float;
	using KPPEvalVectorFloat = EvalVectorFloat[lkpptnum];
	using KKPEvalVectorFloat = EvalVectorFloat[lkkptnum];
	constexpr size_t l_pp_num = fe_end * (fe_end - 1);
	constexpr size_t l_kk_num = SquareNum * (SquareNum - 1);
	constexpr size_t kpptToLkpptnum(const unsigned k, const unsigned p1, const unsigned p2, const unsigned t) { assert(p1 != p2); const size_t p = (p1 > p2) ? (p1 * (p1 - 1) + p2 * 2) : (p2 * (p2 - 1) + p1 * 2); return k * l_pp_num + p + t; }
	constexpr size_t kkptToLkkptnum(const unsigned k1, const unsigned k2, const unsigned p, const unsigned t) { return (size_t)k1 * SquareNum * fe_end * 2 + (size_t)k2 * fe_end * 2 + (size_t)p * 2 + t; }


	class apery_paramVector {
	public:
		static void EvalClamp(std::int16_t absmax);

	public:
		apery_paramVector();
		apery_paramVector(apery_paramVector&&)noexcept;
		apery_paramVector& operator=(apery_paramVector&&)noexcept;
		~apery_paramVector();
		apery_paramVector(const apery_paramVector&) = delete;
		apery_evaluator& operator=(const apery_paramVector&) = delete;

		void reset();
		void addGrad(const float scalar, const SearchPlayer&);
		void clamp(float absmax);

		void updateEval();
		void save(const std::string& path);
		void load(const std::string& path);
	private:
		EvalVectorFloat* KPP;
		EvalVectorFloat* KKP;

	public:
		struct fvpair { const float f; const apery_paramVector& v; fvpair(const float f, const apery_paramVector& v) :f(f), v(v) {} };

	public:
		apery_paramVector& operator+=(const apery_paramVector& rhs);
		apery_paramVector& operator+=(const fvpair& rhs);
		apery_paramVector& operator*=(const double c);

		void showLearnVec_apery(double displaymin, int isKPP)const;

		friend class ShogiTest;
	};
	inline apery_paramVector::fvpair operator*(const float lhs, const apery_paramVector& rhs) { return apery_paramVector::fvpair(lhs, rhs); }



}