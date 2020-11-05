#pragma once
#include "apery_param.h"

namespace apery {
	using EvalElementType = std::array<int16_t, 2>;
	using KPPEvalElementType0 = EvalElementType[fe_end];
	using KPPEvalElementType1 = KPPEvalElementType0[fe_end];
	using KPPEvalElementType2 = KPPEvalElementType1[SquareNum];
	using KKPEvalElementType0 = EvalElementType[fe_end];
	using KKPEvalElementType1 = KKPEvalElementType0[SquareNum];
	using KKPEvalElementType2 = KKPEvalElementType1[SquareNum];

	struct EvalList {
		static constexpr int EvalListSize = 38;
		std::array<EvalIndex, EvalListSize> list0;
		std::array<EvalIndex, EvalListSize> list1;
		int material = 0;

		EvalList() :list0{ {f_hand_pawn} }, list1{ {f_hand_pawn} } {} //search_treeはコンストラクトを持たないのでデフォルトコンストラクタが要る
		EvalList(const Kyokumen& k) { set(k); }
		void set(const Kyokumen&);
	};

	struct EvalSum {
		EvalSum() :p{ {{0,0},{0,0},{0,0}} } {}
		std::array<std::array<std::int32_t, 2>, 3> p;
		std::int32_t sum(bool teban) const {
			std::int32_t BanScore = p[0][0] - p[1][0] + p[2][0];
			std::int32_t TebanScore = p[0][1] + p[1][1] + p[2][1];
			return (teban ? BanScore : -BanScore) + TebanScore;
		}
		EvalSum& operator+=(const EvalSum& rhs) {
			p[0][0] += rhs.p[0][0]; p[0][1] += rhs.p[0][1];
			p[1][0] += rhs.p[1][0];	p[1][1] += rhs.p[1][1];
			p[2][0] += rhs.p[2][0];	p[2][1] += rhs.p[2][1];
			return *this;
		}
		EvalSum& operator-=(const EvalSum& rhs) {
			p[0][0] -= rhs.p[0][0]; p[0][1] -= rhs.p[0][1];
			p[1][0] -= rhs.p[1][0];	p[1][1] -= rhs.p[1][1];
			p[2][0] -= rhs.p[2][0];	p[2][1] -= rhs.p[2][1];
			return *this;
		}
		EvalSum operator+(const EvalSum& rhs) { return EvalSum(*this) += rhs; }
		EvalSum operator-(const EvalSum& rhs) { return EvalSum(*this) -= rhs; }
	};

	inline std::array<std::int32_t, 2> operator += (std::array<std::int32_t, 2> & lhs, const std::array<std::int16_t, 2> & rhs) {
		lhs[0] += rhs[0];
		lhs[1] += rhs[1];
		return lhs;
	}
	inline std::array<std::int32_t, 2> operator -= (std::array<std::int32_t, 2> & lhs, const std::array<std::int16_t, 2> & rhs) {
		lhs[0] -= rhs[0];
		lhs[1] -= rhs[1];
		return lhs;
	}

	class apery_feat {
	private:
		static KPPEvalElementType1* KPP;
		static KKPEvalElementType1* KKP;
		static bool allocated;
	public:
		static void init(const std::string& path);
		static void save(const std::string& path);
		static EvalSum EvalFull(const Kyokumen&, const EvalList&);

	public:
		apery_feat() {}
		apery_feat(const Kyokumen& k) :idlist(k) { sum = EvalFull(k, idlist); }
		EvalList idlist;
		EvalSum sum;
		void set(const Kyokumen& kyokumen);
		void proceed(const Kyokumen& before, const Move& move);
		void recede(const Kyokumen& before,const koma::Koma moved,const koma::Koma captured, const Move move, const EvalSum& cache);
		EvalSum getCache() { return sum; }
		bool operator==(const apery_feat& rhs)const;
		bool operator!=(const apery_feat& rhs)const {
			return !operator==(rhs);
		}
		std::string toString()const;
		friend class apery_paramVector;
	};
}