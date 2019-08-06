#pragma once
#include "koma.h"
#include "bitboard.h"

class Kyokumen {
public:


	koma::Koma getKoma(koma::Position pos) { assert(pos < 81); return static_cast<koma::Koma> (bammen[static_cast<size_t>(pos)]); }
	unsigned getMochiNum(koma::Position mpos) { assert(mpos >= 81); return static_cast<unsigned> (bammen[static_cast<size_t>(mpos)]); }
	unsigned getMochiNum(bool teban, koma::Mochigoma koma) { return teban ? static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 80]) : static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 80 + 7]); }
private:
	std::array<std::uint8_t,95> bammen;
	Bitboard allKomaBB;
	Bitboard senteKomaBB;
	Bitboard goteKomaBB;
	std::array<Bitboard, static_cast<size_t>(koma::Koma::KomaNum)> eachKomaBB;

};