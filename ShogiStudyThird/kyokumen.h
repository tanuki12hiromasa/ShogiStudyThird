#pragma once
#include "koma.h"
#include "bitboard.h"

class Kyokumen {
	friend class BBkiki;
public:
	Kyokumen();
	Kyokumen(const std::array<koma::Koma, 81>&, const std::array<unsigned,7>&, const std::array<unsigned, 7>&, bool);
	Kyokumen(std::array<koma::Koma, 81>&&, std::array<unsigned, 7>&&, std::array<unsigned, 7>&&, bool);
	std::string toSfen()const;
	
	bool teban();
	koma::Koma getKoma(koma::Position pos) { assert(pos < 81); return static_cast<koma::Koma> (bammen[static_cast<size_t>(pos)]); }
	unsigned getMochiNum(koma::Position mpos) { assert(mpos >= 81); return static_cast<unsigned> (bammen[static_cast<size_t>(mpos)]); }
	unsigned getMochiNum(bool teban, koma::Mochigoma koma) { return teban ? static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 80]) : static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 80 + 7]); }
private:
	bool isSente;
	std::array<std::uint8_t,95> bammen;
	Bitboard allKomaBB;
	Bitboard senteKomaBB;
	Bitboard goteKomaBB;
	std::array<Bitboard, static_cast<size_t>(koma::Koma::KomaNum)> eachKomaBB;

};