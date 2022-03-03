#pragma once
#include <cassert>
#include <cstdint>
#include <cmath>
#include <array>

namespace koma {

	//駒を表すenum s:先手 g:後手
	enum class Koma : std::uint8_t {
		s_Fu, s_Kyou, s_Kei, s_Gin, s_Kaku, s_Hi, s_Kin, s_Ou,
		s_nFu, s_nKyou, s_nKei, s_nGin, s_nKaku, s_nHi,
		g_Fu, g_Kyou, g_Kei, g_Gin, g_Kaku, g_Hi, g_Kin, g_Ou,
		g_nFu, g_nKyou, g_nKei, g_nGin, g_nKaku, g_nHi,
		KomaNum,
		None,
		Nari = s_nFu - s_Fu,
		Sengo = g_Fu - s_Fu,
		s_Min = s_Fu, s_Max = s_nHi, s_Num = s_Max + 1,
		g_Min = g_Fu, g_Max = g_nHi, g_Num = g_Max + 1
	};

	//盤上の駒の所属を求める関数
	extern bool isSenteKoma(Koma koma);
	extern bool isGoteKoma(Koma koma);
	//先後の対応した駒を返す
	inline Koma StoG(Koma koma) { return static_cast<Koma>(static_cast<std::uint8_t>(koma) + static_cast<std::uint8_t>(Koma::Sengo)); }
	inline Koma GtoS(Koma koma) { return static_cast<Koma>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::Sengo)); }
	extern std::array<Koma, static_cast<size_t>(Koma::KomaNum)> sgInvArr;
	inline Koma sgInv(Koma koma) { return sgInvArr[static_cast<size_t>(koma)]; }

	//成れる駒かどうか
	extern bool isPromotable(Koma koma);
	//成駒かどうか
	extern bool isPromoted(Koma koma);
	//成れる駒なら成る 成れない場合そのまま返す
	extern Koma promote(Koma koma);
	//成駒なら戻す それ以外はそのまま返す
	extern Koma dispromote(Koma koma);

	//遠距離移動できる駒かどうか
	inline bool isDashable(Koma koma) {
		switch (koma)
		{
		case Koma::s_Kyou:
		case Koma::s_Kaku:
		case Koma::s_Hi:
		case Koma::s_nHi:
		case Koma::s_nKaku:
		case Koma::g_Kyou:
		case Koma::g_Kaku:
		case Koma::g_Hi:
		case Koma::g_nKaku:
		case Koma::g_nHi:
			return true;
		default:
			return false;
		}
	}
	//1マス移動できる駒かどうか(桂もこの部類に入る)
	inline bool isSteppable(Koma koma) {
		switch (koma)
		{
		case Koma::s_Kyou:
		case Koma::s_Kaku:
		case Koma::s_Hi:
		case Koma::g_Kyou:
		case Koma::g_Kaku:
		case Koma::g_Hi:
			return false;
		default:
			return true;
		}
	}

	//持ち駒を表すenum 
	enum class Mochigoma : std::uint8_t {
		Fu, Kyou, Kei, Gin, Kaku, Hi, Kin, MochigomaNum, None
	};

 	extern Mochigoma KomaToMochi(const Koma koma);
	extern Koma MochiToKoma(const Mochigoma mochi, const bool sengo);

	//将棋盤の1一から1九、2一から...って感じの並びになっている(π/2傾いていると思えば)
	enum Position : std::uint8_t {
		SQ11, SQ12, SQ13, SQ14, SQ15, SQ16, SQ17, SQ18, SQ19,
		SQ21, SQ22, SQ23, SQ24, SQ25, SQ26, SQ27, SQ28, SQ29,
		SQ31, SQ32, SQ33, SQ34, SQ35, SQ36, SQ37, SQ38, SQ39,
		SQ41, SQ42, SQ43, SQ44, SQ45, SQ46, SQ47, SQ48, SQ49,
		SQ51, SQ52, SQ53, SQ54, SQ55, SQ56, SQ57, SQ58, SQ59,
		SQ61, SQ62, SQ63, SQ64, SQ65, SQ66, SQ67, SQ68, SQ69,
		SQ71, SQ72, SQ73, SQ74, SQ75, SQ76, SQ77, SQ78, SQ79,
		SQ81, SQ82, SQ83, SQ84, SQ85, SQ86, SQ87, SQ88, SQ89,
		SQ91, SQ92, SQ93, SQ94, SQ95, SQ96, SQ97, SQ98, SQ99,

		m_sFu, m_sKyou, m_sKei, m_sGin, m_sKaku, m_sHi, m_sKin,
		m_gFu, m_gKyou, m_gKei, m_gGin, m_gKaku, m_gHi, m_gKin,

		SQMin = SQ11, SQMax = SQ99,
		SQNum = SQMax + 1,
		SQm_Min = m_sFu, SQm_Max = m_gKin,
		SQm_Num = SQm_Max + 1,
		NullMove
	};
	enum Dan : int {
		Dan1 = 0, Dan2, Dan3, Dan4, Dan5, Dan6, Dan7, Dan8, Dan9
	};
	extern const std::array<int, 81> SQDan;
	//座標を段に変換する関数
	inline int PosToDan(const int pos) {
		return SQDan[pos];
	}

	enum Suji :int {
		Suji1 = 0, Suji2, Suji3, Suji4, Suji5, Suji6, Suji7, Suji8, Suji9
	};
	extern const std::array<int, 81> SQSuji;
	//座標を筋に変換する関数
	inline int PosToSuji(const int pos) {
		return SQSuji[pos];
	}

	//x,yはそれぞれ[0,8]
	inline unsigned XYtoPos(const unsigned x, const unsigned y) {
		return x * 9 + y;
	}
	inline int XYtoPos(const int x, const int y) {
		assert(0 <= x && x < 9 && 0 <= y && y < 9);
		return x * 9 + y;
	}
	inline int mirrorX(const int& pos) {
		int x = (int)pos / 5; int y = (int)pos - x * 5;
		return XYtoPos(4 - x, y);
	}
	inline Position mirrorX(const Position& pos) {
		if (pos > Position::SQMax)return pos;
		else return (Position)mirrorX((int)pos);
	}

	inline bool isInside(const int pos) {
		return pos >= Position::SQMin && pos < Position::SQNum;
	}

	//position内の持ち駒をKomaに変換する関数
	extern Koma MposToKoma(const Position pos);
	extern Mochigoma MposToMochi(const Position pos);
	//Komaをposition内の持ち駒に変換する関数(取られるので所属が変わることに注意)
	extern Position KomaToMpos(Koma koma);
	extern Position MochiToMpos(Mochigoma koma, bool sente);
}