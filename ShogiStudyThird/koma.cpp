#include "stdafx.h"
#include "koma.h"

namespace koma {
	const std::array<int, 81> SQDan = {
		Dan1,Dan2,Dan3,Dan4,Dan5,Dan6,Dan7,Dan8,Dan9,
		Dan1,Dan2,Dan3,Dan4,Dan5,Dan6,Dan7,Dan8,Dan9,
		Dan1,Dan2,Dan3,Dan4,Dan5,Dan6,Dan7,Dan8,Dan9,
		Dan1,Dan2,Dan3,Dan4,Dan5,Dan6,Dan7,Dan8,Dan9,
		Dan1,Dan2,Dan3,Dan4,Dan5,Dan6,Dan7,Dan8,Dan9,
		Dan1,Dan2,Dan3,Dan4,Dan5,Dan6,Dan7,Dan8,Dan9,
		Dan1,Dan2,Dan3,Dan4,Dan5,Dan6,Dan7,Dan8,Dan9,
		Dan1,Dan2,Dan3,Dan4,Dan5,Dan6,Dan7,Dan8,Dan9,
		Dan1,Dan2,Dan3,Dan4,Dan5,Dan6,Dan7,Dan8,Dan9,
	};

	const std::array<int, 81> SQSuji = {
		Suji1,Suji1,Suji1,Suji1,Suji1,Suji1,Suji1,Suji1,Suji1,
		Suji2,Suji2,Suji2,Suji2,Suji2,Suji2,Suji2,Suji2,Suji2,
		Suji3,Suji3,Suji3,Suji3,Suji3,Suji3,Suji3,Suji3,Suji3,
		Suji4,Suji4,Suji4,Suji4,Suji4,Suji4,Suji4,Suji4,Suji4,
		Suji5,Suji5,Suji5,Suji5,Suji5,Suji5,Suji5,Suji5,Suji5,
		Suji6,Suji6,Suji6,Suji6,Suji6,Suji6,Suji6,Suji6,Suji6,
		Suji7,Suji7,Suji7,Suji7,Suji7,Suji7,Suji7,Suji7,Suji7,
		Suji8,Suji8,Suji8,Suji8,Suji8,Suji8,Suji8,Suji8,Suji8,
		Suji9,Suji9,Suji9,Suji9,Suji9,Suji9,Suji9,Suji9,Suji9
	};

	bool isSenteKoma(Koma koma) {
		return static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::s_Min) &&
			static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::s_Max);
	}
	bool isGoteKoma(Koma koma) {
		return static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::g_Min) &&
			static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::g_Max);
	}
	std::array<Koma, static_cast<size_t>(Koma::KomaNum)> sgInvArr = {
		Koma::g_Fu, Koma::g_Kyou, Koma::g_Kei, Koma::g_Gin, Koma::g_Kaku, Koma::g_Hi, Koma::g_Kin, Koma::g_Ou,
		Koma::g_nFu,Koma::g_nKyou,Koma::g_nKei,Koma::g_nGin,Koma::g_nKaku,Koma::g_nHi,
		Koma::s_Fu, Koma::s_Kyou, Koma::s_Kei, Koma::s_Gin, Koma::s_Kaku, Koma::s_Hi, Koma::s_Kin, Koma::s_Ou,
		Koma::s_nFu,Koma::s_nKyou,Koma::s_nKei,Koma::s_nGin,Koma::s_nKaku,Koma::s_nHi
	};
	bool isPromotable(Koma koma) {
		return (static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::s_Hi) &&
			static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::s_Fu) ||
			static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::g_Hi) &&
			static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::g_Fu));
	}
	bool isPromoted(Koma koma) {
		return (static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::s_nHi) &&
			static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::s_nFu) ||
			static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::g_nFu)) &&
			static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::g_nHi);
	}
	Koma promote(Koma koma) {
		assert(isPromotable(koma));
		return static_cast<Koma>(static_cast<std::uint8_t>(koma) + static_cast<std::uint8_t>(Koma::Nari));
	}
	Koma dispromote(Koma koma) {
		if (isPromoted(koma)) {
			return static_cast<Koma>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::Nari));
		}
		else return koma;
	}

	Mochigoma KomaToMochi(const Koma koma) {
		assert(koma != Koma::s_Ou && koma != Koma::g_Ou);//王は持ち駒にならない
		if (koma == Koma::None) {//None
			return Mochigoma::None;
		}
		else if (static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::s_Max)) {//先手駒
			if (static_cast<std::uint8_t>(koma) < static_cast<std::uint8_t>(Koma::s_Ou)) {//通常駒
				return static_cast<Mochigoma>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::s_Min));
			}
			else {//成駒
				return static_cast<Mochigoma>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::s_Min) - static_cast<std::uint8_t>(Koma::Nari));
			}
		}
		else {//後手駒
			if (static_cast<std::uint8_t>(koma) < static_cast<std::uint8_t>(Koma::g_Ou)) {//通常駒
				return static_cast<Mochigoma>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::g_Min));
			}
			else {//成駒
				return static_cast<Mochigoma>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::g_Min) - static_cast<std::uint8_t>(Koma::Nari));
			}
		}
	}

	Koma MochiToKoma(const Mochigoma mochi, const bool sengo) {
		assert(static_cast<std::uint8_t>(mochi) < static_cast<std::uint8_t>(Mochigoma::MochigomaNum));
		if (mochi == Mochigoma::None) {//None
			return Koma::None;
		}
		else if (sengo) {//先手
			return static_cast<Koma>(static_cast<std::uint8_t>(mochi) + static_cast<std::uint8_t>(Koma::s_Min));
		}
		else {//後手
			return static_cast<Koma>(static_cast<std::uint8_t>(mochi) + static_cast<std::uint8_t>(Koma::g_Min));
		}
	}
	Koma MposToKoma(const Position pos) {
		assert(pos >= Position::SQm_Min && pos <= Position::SQm_Max);
		return (pos < Position::m_gFu) ?
			static_cast<Koma>(static_cast<std::uint8_t>(pos) - static_cast<std::uint8_t>(m_sFu) + static_cast<std::uint8_t>(Koma::s_Fu)) :
			static_cast<Koma>(static_cast<std::uint8_t>(pos) - static_cast<std::uint8_t>(m_gFu) + static_cast<std::uint8_t>(Koma::g_Fu));
	}
	
	Mochigoma MposToMochi(const Position pos) {
		assert(pos >= Position::SQm_Min && pos <= Position::SQm_Max);
		return (pos < Position::m_gFu) ?
			static_cast<Mochigoma>(pos - m_sFu) :
			static_cast<Mochigoma>(pos - m_gFu);
	}

	Position KomaToMpos(Koma koma) {
		if (isPromoted(koma)) { koma = dispromote(koma); }
		return (static_cast<int>(koma) < static_cast<int>(Koma::g_Min)) ?
			static_cast<Position>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::s_Fu) + static_cast<std::uint8_t>(Position::m_gFu)) :
			static_cast<Position>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::g_Fu) + static_cast<std::uint8_t>(Position::m_sFu));
	}
	Position MochiToMpos(Mochigoma koma, bool sente) {
		if (sente) {
			return static_cast<Position>(static_cast<std::uint8_t>(koma) + static_cast<std::uint8_t>(Position::m_sFu));
		}
		else {
			return static_cast<Position>(static_cast<std::uint8_t>(koma) + static_cast<std::uint8_t>(Position::m_gFu));
		}
	}

}