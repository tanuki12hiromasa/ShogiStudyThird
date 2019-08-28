#pragma once
#include "bitboard.h"
#include "koma.h"
#include <array>

class BBkiki {
public:
	static void init();
	static const Bitboard getStepKiki(const koma::Koma koma, const unsigned komapos);
	static const Bitboard getDashKiki(const Bitboard& allbb, const koma::Koma koma, const unsigned komapos);
	static const Bitboard getKiki(const Bitboard& allbb, const koma::Koma koma, const unsigned komapos);
private:
	static const Bitboard getSKyouDashKiki(const Bitboard& allbb, const unsigned komapos);
	static const Bitboard getGKyouDashKiki(const Bitboard& allbb, const unsigned komapos);
	static const Bitboard getHiDashKiki(const Bitboard& allbb, const unsigned komapos);
	static const Bitboard getKakuDashKiki(const Bitboard& allbb, const unsigned komapos);

public:
	using LBarray81 = std::array<LittleBitboard, 81>;
private:
	static LBarray81 sFu;
	static LBarray81 sKei;
	static LBarray81 sGin;
	static LBarray81 sKin;
	static LBarray81 gFu;
	static LBarray81 gKei;
	static LBarray81 gGin;
	static LBarray81 gKin;
	static LBarray81 UmaStep;
	static LBarray81 RyuStep;
	static LBarray81 Ou;

	static std::array<std::uint64_t, 81> KakuPositiveInclinationMask;//角の斜めは、傾きの正負で名前を付けている
	static std::array<std::uint64_t, 81> KakuNegativeInclinationMask;
	static std::array<std::uint64_t, 9> HiVerticalMask;
	static std::array<std::uint64_t, 9> HiHorizontalMask;
	static std::array<std::uint64_t, 9> sKyouMask;//一番上には居れないので8通りになる
	static std::array<std::uint64_t, 9> gKyouMask;

	static std::array<unsigned, 81> KPIndex;
	static std::array<unsigned, 81> KNIndex;
	static std::array<unsigned, 9> HIndex;//飛車はのindexは一つで十分
	static std::array<unsigned, 9> sKIndex;
	static std::array<unsigned, 9> gKIndex;

	static std::array<LittleBitboard, 1856> KakuDashPositive;//初期位置で角が睨み合う方の向き
	static std::array<LittleBitboard, 1856> KakuDashNegative;
	static std::array<std::uint64_t, 704> HiDashVertical;//縦
	static std::array<LittleBitboard, 704> HiDashHorizontal;//横
	static std::array<std::uint64_t, 256> sKyouDash;
	static std::array<std::uint64_t, 256> gKyouDash;

private:
	static void genData();//利きテーブルを生成する(事前に実行して準備しておく)
	static void genMask();
	static void genIndex();
	static void genDashTable();
	static void genStepTable();
};

BBkiki::LBarray81 BBkiki::sFu;
BBkiki::LBarray81 BBkiki::sKei;
BBkiki::LBarray81 BBkiki::sGin;
BBkiki::LBarray81 BBkiki::sKin;
BBkiki::LBarray81 BBkiki::gFu;
BBkiki::LBarray81 BBkiki::gKei;
BBkiki::LBarray81 BBkiki::gGin;
BBkiki::LBarray81 BBkiki::gKin;
BBkiki::LBarray81 BBkiki::UmaStep;
BBkiki::LBarray81 BBkiki::RyuStep;
BBkiki::LBarray81 BBkiki::Ou;
std::array<std::uint64_t, 81> BBkiki::KakuPositiveInclinationMask;
std::array<std::uint64_t, 81> BBkiki::KakuNegativeInclinationMask;
std::array<std::uint64_t, 9> BBkiki::HiVerticalMask;
std::array<std::uint64_t, 9> BBkiki::HiHorizontalMask;
std::array<std::uint64_t, 9> BBkiki::sKyouMask;
std::array<std::uint64_t, 9> BBkiki::gKyouMask;
std::array<unsigned, 81> BBkiki::KPIndex;
std::array<unsigned, 81> BBkiki::KNIndex;
std::array<unsigned, 9> BBkiki::HIndex;
std::array<unsigned, 9> BBkiki::sKIndex;
std::array<unsigned, 9> BBkiki::gKIndex;
std::array<LittleBitboard, 1856> BBkiki::KakuDashPositive;
std::array<LittleBitboard, 1856> BBkiki::KakuDashNegative;
std::array<std::uint64_t, 704> BBkiki::HiDashVertical;
std::array<LittleBitboard, 704> BBkiki::HiDashHorizontal;
std::array<std::uint64_t, 256> BBkiki::sKyouDash;
std::array<std::uint64_t, 256> BBkiki::gKyouDash;


inline const Bitboard BBkiki::getStepKiki(const koma::Koma fromkoma, const unsigned komapos) {
	using namespace koma;
	switch (fromkoma)
	{
	case Koma::s_Fu:
		return Bitboard(sFu[komapos]);
	case Koma::s_Kei:
		return Bitboard(sKei[komapos]);
	case Koma::s_Gin:
		return Bitboard(sGin[komapos]);
	case Koma::s_Kin:
	case Koma::s_nFu:
	case Koma::s_nKyou:
	case Koma::s_nKei:
	case Koma::s_nGin:
		return Bitboard(sKin[komapos]);
	case Koma::g_Fu:
		return Bitboard(gFu[komapos]);
	case Koma::g_Kei:
		return Bitboard(gKei[komapos]);
	case Koma::g_Gin:
		return Bitboard(gGin[komapos]);
	case Koma::g_Kin:
	case Koma::g_nFu:
	case Koma::g_nKyou:
	case Koma::g_nKei:
	case Koma::g_nGin:
		return Bitboard(gKin[komapos]);
	case Koma::s_nKaku:
	case Koma::g_nKaku:
		return Bitboard(UmaStep[komapos]);
	case Koma::s_nHi:
	case Koma::g_nHi:
		return Bitboard(RyuStep[komapos]);
	case Koma::s_Ou:
	case Koma::g_Ou:
		return Bitboard(Ou[komapos]);
	default:
		assert(0);
		return bbmask::AllZero;
	}
}

inline const Bitboard BBkiki::getDashKiki(const Bitboard& allBB, const koma::Koma dkoma, const unsigned komapos) {
	using namespace koma;
	switch (dkoma) {
	case Koma::s_Kyou:
		return getSKyouDashKiki(allBB, komapos);
	case Koma::g_Kyou:
		return getGKyouDashKiki(allBB, komapos);
	case Koma::s_Hi:
	case Koma::g_Hi:
		return getHiDashKiki(allBB, komapos);
	case Koma::s_nHi:
	case Koma::g_nHi:
		return getHiDashKiki(allBB, komapos) | Bitboard(RyuStep[komapos]);
	case Koma::s_Kaku:
	case Koma::g_Kaku:
		return getKakuDashKiki(allBB, komapos);
	case Koma::s_nKaku:
	case Koma::g_nKaku:
		return getKakuDashKiki(allBB, komapos) | Bitboard(UmaStep[komapos]);
	default:
		assert(0);
		return bbmask::AllZero;
	}
}


inline const Bitboard BBkiki::getKiki(const Bitboard& allBB, const koma::Koma koma, const unsigned komapos) {
	using namespace koma;
	switch (koma)
	{
	case Koma::s_Fu:
		return Bitboard(sFu[komapos]);
	case Koma::s_Kei:
		return Bitboard(sKei[komapos]);
	case Koma::s_Gin:
		return Bitboard(sGin[komapos]);
	case Koma::s_Kin:
	case Koma::s_nFu:
	case Koma::s_nKyou:
	case Koma::s_nKei:
	case Koma::s_nGin:
		return Bitboard(sKin[komapos]);
	case Koma::g_Fu:
		return Bitboard(gFu[komapos]);
	case Koma::g_Kei:
		return Bitboard(gKei[komapos]);
	case Koma::g_Gin:
		return Bitboard(gGin[komapos]);
	case Koma::g_Kin:
	case Koma::g_nFu:
	case Koma::g_nKyou:
	case Koma::g_nKei:
	case Koma::g_nGin:
		return Bitboard(gKin[komapos]);
	case Koma::s_Ou:
	case Koma::g_Ou:
		return Bitboard(Ou[komapos]);
	case Koma::s_Kyou:
		return getSKyouDashKiki(allBB, komapos);
	case Koma::g_Kyou:
		return getGKyouDashKiki(allBB, komapos);
	case Koma::s_Hi:
	case Koma::g_Hi:
		return getHiDashKiki(allBB, komapos);
	case Koma::s_nHi:
	case Koma::g_nHi:
		return getHiDashKiki(allBB, komapos) | Bitboard(RyuStep[komapos]);
	case Koma::s_Kaku:
	case Koma::g_Kaku:
		return getKakuDashKiki(allBB, komapos);
	case Koma::s_nKaku:
	case Koma::g_nKaku:
		return getKakuDashKiki(allBB, komapos) | Bitboard(UmaStep[komapos]);
	default:
		assert(0);
		return bbmask::AllZero;
	}
}