#pragma once
#include <array>
#include <cstdint>
#include <string>

//利きテーブルなどで大量のBBを扱う為に省メモリ化したBB
struct LittleBitboard {
	std::uint64_t center;
	std::array<std::uint16_t, 2> side;

	LittleBitboard() :center(0), side({ 0,0 }) {}
	LittleBitboard(std::uint16_t rs, std::uint64_t c, std::uint16_t ls)
		:center(c), side({ rs,ls }) {}
	bool operator==(const LittleBitboard& rhs) {
		return center == rhs.center && side == rhs.side;
	}
};

class Bitboard {
private:
	static const unsigned bbNum = 81;
public:
	Bitboard() :_p({ 0,0,0 }) {}
	Bitboard(std::uint64_t rside, std::uint64_t center, std::uint64_t lside)
		: _p({ rside & 0x01FFULL, center & 0x7FFFFFFFFFFFFFFFULL, lside & 0x01FFULL }){} //不使用ビットが1にならないようにマスクする
	Bitboard(std::array<std::uint64_t, 3> && val): _p(std::move(val))
		{_p[0] &= 0x1FFULL; _p[1] &= 0x7FFFFFFFFFFFFFFFULL; _p[2] &= 0x1FFULL;}
	Bitboard(const unsigned pos) : Bitboard() {	set(pos);}
	Bitboard(const LittleBitboard& lb);
	Bitboard(const std::string& bits);
	LittleBitboard toLittle();
	std::string toString();

	//bit操作・観測
	unsigned pop_first();//先頭の1のbitを探してindexを返し、0にする
	unsigned find_first()const;	//bitを順に探す 見つからなければbbNumを返す
	unsigned find_next(const unsigned)const;
	unsigned find_last()const;
	unsigned popcount()const;
	bool test(const unsigned pos)const;
	void set(const unsigned pos, const bool value);
	void set(const unsigned pos);
	void reset(const unsigned pos);
	void all_reset();
	bool none()const;//すべて0ならtrue
	bool any()const { return !none(); }
	Bitboard getLineOR()const;//1のある筋を全て1で埋めたBBを返す
	Bitboard getNoFuLines()const;//1のある筋を0でマスクしたBBを返す
	static Bitboard fillOne(const unsigned numofone);//左からnumofone個の1で埋めたBBを返す

	//演算子
	bool operator==(const Bitboard&)const;
	bool operator!=(const Bitboard&)const;
	Bitboard operator&(const Bitboard&)const;
	Bitboard operator|(const Bitboard&)const;
	Bitboard& operator&=(const Bitboard&);
	Bitboard& operator|=(const Bitboard&);
	Bitboard operator~()const;
	size_t size() const { return bbNum; }

	const std::array<uint64_t, 3>& val()const { return _p; }
private:
	std::array<uint64_t, 3> _p;

	friend class BBkiki;
};

namespace bbmask {
	static const Bitboard Dan1to8{ 0b011111111ULL,0b011111111011111111011111111011111111011111111011111111011111111ULL,0b011111111ULL };//"011111111011111111011111111011111111011111111011111111011111111011111111011111111"
	static const Bitboard Dan1to7{ 0b001111111ULL,0b001111111001111111001111111001111111001111111001111111001111111ULL,0b001111111ULL };//"001111111001111111001111111001111111001111111001111111001111111001111111001111111"
	static const Bitboard Dan1to6{ 0b000111111ULL,0b000111111000111111000111111000111111000111111000111111000111111ULL,0b000111111ULL };
	static const Bitboard Dan1to5{ 0b000011111ULL,0b000011111000011111000011111000011111000011111000011111000011111ULL,0b000011111ULL };
	static const Bitboard Dan1to4{ 0b000001111ULL,0b000001111000001111000001111000001111000001111000001111000001111ULL,0b000001111ULL };
	static const Bitboard Dan1to3{ 0b000000111ULL,0b000000111000000111000000111000000111000000111000000111000000111ULL,0b000000111ULL };
	static const Bitboard Dan2to9{ 0b111111110ULL,0b111111110111111110111111110111111110111111110111111110111111110ULL,0b111111110ULL };//"111111110111111110111111110111111110111111110111111110111111110111111110111111110"
	static const Bitboard Dan3to9{ 0b111111100ULL,0b111111100111111100111111100111111100111111100111111100111111100ULL,0b111111100ULL };//"111111100111111100111111100111111100111111100111111100111111100111111100111111100"
	static const Bitboard Dan4to9{ 0b111111000ULL,0b111111000111111000111111000111111000111111000111111000111111000ULL,0b111111000ULL };
	static const Bitboard Dan5to9{ 0b111110000ULL,0b111110000111110000111110000111110000111110000111110000111110000ULL,0b111110000ULL };
	static const Bitboard Dan6to9{ 0b111100000ULL,0b111100000111100000111100000111100000111110000111100000111100000ULL,0b111100000ULL };
	static const Bitboard Dan7to9{ 0b111000000ULL,0b111000000111000000111000000111000000111000000111000000111000000ULL,0b111000000ULL };
	static const Bitboard Dan3   { 0b000000100ULL,0b000000100000000100000000100000000100000000100000000100000000100ULL,0b000000100ULL };
	static const Bitboard Dan7   { 0b001000000ULL,0b001000000001000000001000000001000000001000000001000000001000000ULL,0b001000000ULL };
	static const Bitboard AllOne{ 0b111111111ULL,0b111111111111111111111111111111111111111111111111111111111111111ULL,0b111111111ULL };//"111111111111111111111111111111111111111111111111111111111111111111111111111111111"
	static const Bitboard AllZero{ 0,0,0 };
}


inline bool Bitboard::operator==(const Bitboard& rhs) const {
	return _p == rhs._p;
}

inline bool Bitboard::operator!=(const Bitboard& rhs)const {
	return _p != rhs._p;
}

inline Bitboard Bitboard::operator&(const Bitboard& rhs) const {
	return Bitboard(_p[0] & rhs._p[0], _p[1] & rhs._p[1], _p[2] & rhs._p[2]);
}

inline Bitboard Bitboard::operator|(const Bitboard& rhs) const {
	return Bitboard(_p[0] | rhs._p[0], _p[1] | rhs._p[1], _p[2] | rhs._p[2]);
}

inline Bitboard& Bitboard::operator&=(const Bitboard& rhs) {
	_p[0] &= rhs._p[0]; _p[1] &= rhs._p[1]; _p[2] &= rhs._p[2];
	return *this;
}

inline Bitboard& Bitboard::operator|=(const Bitboard& rhs) {
	_p[0] |= rhs._p[0]; _p[1] |= rhs._p[1]; _p[2] |= rhs._p[2];
	return *this;
}

inline Bitboard Bitboard::operator~()const {
	//そのまま反転させると不使用ビットが1になってしまうが、コンストラクタ内でマスクして0になるので大丈夫
	return Bitboard(~_p[0], ~_p[1], ~_p[2]);
}