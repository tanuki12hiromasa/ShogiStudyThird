﻿#pragma once
#include "kppt_evaluate.h"
#include <cassert>
#include <functional>

namespace kppt {
	constexpr size_t lkpptnum = SquareNum * fe_end * (fe_end - 1); //kpptテーブルのppの被りを消した三角テーブルを一次元にしている セル数はSQnum*(fe*(fe-1)/2)*2
	constexpr size_t lkkptnum = SquareNum * SquareNum * fe_end * 2;//こちらは先後の区別があるため単純に一次元にしている
	using EvalVectorFloat = float;
	using KPPEvalVectorFloat = EvalVectorFloat[lkpptnum];
	using KKPEvalVectorFloat = EvalVectorFloat[lkkptnum];
	constexpr size_t l_pp_num = fe_end * (fe_end - 1);
	constexpr size_t l_kk_num = SquareNum * (SquareNum - 1);
	constexpr size_t kpptToLkpptnum(const unsigned k, const unsigned p1, const unsigned p2, const unsigned t) { assert(p1 != p2); const size_t p = (p1 > p2) ? (p1 * (p1 - 1) + p2 * 2) : (p2 * (p2 - 1) + p1 * 2); return k * l_pp_num + p + t; }
	constexpr size_t kkptToLkkptnum(const unsigned k1, const unsigned k2, const unsigned p, const unsigned t) { return (size_t)k1 * SquareNum * fe_end * 2 + (size_t)k2 * fe_end * 2 + (size_t)p * 2 + t; }

	class kppt_paramVector {
	public:
		static void EvalClamp(std::int16_t absmax);

	public:
		kppt_paramVector();
		kppt_paramVector(kppt_paramVector&&)noexcept;
		kppt_paramVector& operator=(kppt_paramVector&&)noexcept;
		~kppt_paramVector();
		kppt_paramVector(const kppt_paramVector&) = delete;
		kppt_evaluator& operator=(const kppt_paramVector&) = delete;

		void reset();
		void addGrad(const float scalar, const SearchPlayer&);
		void clamp(float absmax);

		void updateEval();
		void save(const std::string& path);//勾配ファイルを出力
		void load(const std::string& path);//勾配ファイルを読み込む
	private:
		EvalVectorFloat* KPP;
		EvalVectorFloat* KKP;

	public:
		struct fvpair { const float f; const kppt_paramVector& v; fvpair(const float f, const kppt_paramVector& v) :f(f), v(v) {} };

	public:
		kppt_paramVector& operator+=(const kppt_paramVector& rhs);
		kppt_paramVector& operator+=(const fvpair& rhs);
		kppt_paramVector& operator*=(const double c);

		void print(double displaymin, int isKPP)const;

		friend class kppt_learn;
		friend class ShogiTest;
	};
	inline kppt_paramVector::fvpair operator*(const float lhs, const kppt_paramVector& rhs) { return kppt_paramVector::fvpair(lhs, rhs); }

}
