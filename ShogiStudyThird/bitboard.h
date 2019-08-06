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
	Bitboard(std::uint64_t, std::uint64_t, std::uint64_t);
	Bitboard(std::array<std::uint64_t, 3>&&);
	Bitboard(const LittleBitboard& lb);
	Bitboard(const std::string& bits);
	LittleBitboard toLittle();
	std::string toString();

	//bit操作・観測
	unsigned pop_first();//先頭の1のbitを探してindexを返し、0にする
	unsigned find_first()const;	//bitを順に探す 見つからなければbbNumを返す
	unsigned find_next(const unsigned)const;
	unsigned popcount()const;
	bool test(const unsigned pos)const;
	void set(const unsigned pos, const bool value);
	void set(const unsigned pos);
	void reset(const unsigned pos);
	void all_reset();
	bool none()const;//すべて0ならtrue
	bool any()const { return !none(); }
	Bitboard getLineOR()const;//1のある筋を全て1で埋めたBBを返す
	static Bitboard fillOne(const unsigned numofone);//左からnumofone個の1で埋めたBBを返す

	//演算子
	bool operator==(const Bitboard&)const;
	Bitboard operator&(const Bitboard&)const;
	Bitboard operator|(const Bitboard&)const;
	Bitboard& operator&=(const Bitboard&);
	Bitboard& operator|=(const Bitboard&);
	Bitboard operator~()const;
	size_t size() const { return bbNum; }

private:
	std::array<uint64_t, 3> _p;

	friend class BBkiki;
};

namespace bbmask {
	static const Bitboard Dan1to8{ 0b011111111ULL,0b011111111011111111011111111011111111011111111011111111011111111ULL,0b011111111ULL };//"011111111011111111011111111011111111011111111011111111011111111011111111011111111"
	static const Bitboard Dan1to7{ 0b001111111ULL,0b001111111001111111001111111001111111001111111001111111001111111ULL,0b001111111ULL };//"001111111001111111001111111001111111001111111001111111001111111001111111001111111"
	static const Bitboard Dan2to9{ 0b111111110ULL,0b111111110111111110111111110111111110111111110111111110111111110ULL,0b111111110ULL };//"111111110111111110111111110111111110111111110111111110111111110111111110111111110"
	static const Bitboard Dan3to9{ 0b111111100ULL,0b111111100111111100111111100111111100111111100111111100111111100ULL,0b111111100ULL };//"111111100111111100111111100111111100111111100111111100111111100111111100111111100"
	static const Bitboard AllOne{ 0b111111111ULL,0b111111111111111111111111111111111111111111111111111111111111111ULL,0b111111111ULL };//"111111111111111111111111111111111111111111111111111111111111111111111111111111111"
	static const Bitboard AllZero{ 0,0,0 };
}