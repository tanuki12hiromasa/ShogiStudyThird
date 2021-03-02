#pragma once
#include "koma.h"
#include <vector>
#include <string>

class Move {
public:
	static std::vector<Move> usiToMoves(const std::vector<std::string>& tokens);

	Move() :Move(koma::Position::NullMove, koma::Position::NullMove, false) {}

	Move(std::uint8_t from, std::uint8_t to, bool promote)
		: u((from & 0x7Fu) | ((to & 0x7Fu) << 7) | (promote << 14))
	{}
	Move(koma::Position from,koma::Position to, bool promote)
		:Move(static_cast<std::uint8_t>(from), static_cast<std::uint8_t>(to),promote)
	{}
	Move(const std::string& usi, bool teban)
		:Move(usiToFrom(usi,teban),usiToTo(usi),usiToPromote(usi))
	{}

	Move(std::uint16_t _u) { u = _u; }

	void setOute(bool isOute) { u = (u & 0x7FFFu) | (isOute << 15); }

	koma::Position from()const { return static_cast<koma::Position>(u & 0x7Fu); }
	koma::Position to()const { return static_cast<koma::Position>((u >> 7) & 0x7Fu); }
	bool promote()const { return static_cast<bool>((u >> 14) & 0x1u); }
	bool isOute()const { return static_cast<bool>((u >> 15) & 0x1u); }
	std::string toUSI()const;

	bool operator==(const Move& rhs) const {
		return (u & 0x7FFFu) == (rhs.u & 0x7FFFu);
	}
	bool operator!=(const Move& rhs) const { return !operator==(rhs); }

	std::uint16_t getU() const { return u; }
private:
	static unsigned usiToFrom(const std::string& usi, bool teban);
	static unsigned usiToTo(const std::string& usi);
	static bool usiToPromote(const std::string& usi);
	//メモリ節約のため16bitに収めている
	std::uint16_t u;//0-6 from, 8-13 to,14 promote,15 isOute
};

inline unsigned Move::usiToFrom(const std::string& usi, bool teban) {
	if (usi[1] == '*') { //2文字目が*なら打ち駒
		unsigned from = static_cast<unsigned>(teban ? koma::Position::m_sFu : koma::Position::m_gFu);
		switch (usi[0])
		{
		case 'P': return from + 0;
		case 'L': return from + 1;
		case 'N': return from + 2;
		case 'S': return from + 3;
		case 'B': return from + 4;
		case 'R': return from + 5;
		case 'G': return from + 6;
		default:assert(0); return 0;
			break;
		}
	}
	else {
		return (usi[0] - '1') * 9 + (usi[1] - 'a'); //1文字目が筋 2文字目が段
	}
}

inline unsigned Move::usiToTo(const std::string& usi) {
	return (usi[2] - '1') * 9 + (usi[3] - 'a'); //3文字目が筋 4文字目が段
}

inline bool Move::usiToPromote(const std::string& usi) {
	return (usi[4] == '+');//5文字目が+なら成り
}