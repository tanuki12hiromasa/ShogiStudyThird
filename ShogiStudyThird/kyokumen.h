#pragma once
#include "koma.h"
#include "bitboard.h"
#include "move.h"
#include "vec2.h"

class Kyokumen {
	friend class BBkiki;
	friend class SearchPlayer;
public:
	Kyokumen():Kyokumen(prime_bammen, true) {}
	Kyokumen(const std::array<koma::Koma, 81>&, const std::array<unsigned,7>&, const std::array<unsigned, 7>&, bool);
	Kyokumen(const std::array<std::uint8_t, 95>&, bool);
	Kyokumen(std::array<std::uint8_t, 95>&&, bool);
	Kyokumen(const std::vector<std::string>& usitokens);
	std::string toSfen()const;
	
	koma::Position proceed(const Move);

	bool teban()const { return isSente; }
	koma::Koma getKoma(const koma::Position pos)const { assert(pos < 81); return static_cast<koma::Koma> (bammen[static_cast<size_t>(pos)]); }
	koma::Koma getKoma(const unsigned pos)const { assert(pos < 81); return static_cast<koma::Koma> (bammen[static_cast<size_t>(pos)]); }
	unsigned getMochigomaNum(koma::Position mpos) { assert(mpos >= 81); return static_cast<unsigned> (bammen[static_cast<size_t>(mpos)]); }
	unsigned getMochigomaNum(bool teban, koma::Mochigoma koma) { return teban ? static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 81]) : static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 81 + 7]); }
	const Bitboard& getAllBB()const { return allKomaBB; }
	const Bitboard& getSenteBB()const { return senteKomaBB; }
	const Bitboard& getGoteBB()const { return goteKomaBB; }
	const Bitboard& getEachBB(const koma::Koma k)const { return eachKomaBB[static_cast<size_t>(k)]; }
	unsigned sOuPos()const { return eachKomaBB[static_cast<size_t>(koma::Koma::s_Ou)].find_first(); }
	unsigned gOuPos()const { return eachKomaBB[static_cast<size_t>(koma::Koma::g_Ou)].find_first(); }
	
	bool isSenteChecked(const Move)const;
	bool isGoteChecked(const Move)const;
	std::vector<Bitboard> getSenteOuCheck(const Move)const;
	std::vector<Bitboard> getGoteOuCheck(const Move)const;

	Bitboard pinMaskSente(const unsigned pos)const;//pos上の駒が敵駒にpinされていれば移動可能な直線範囲を返す pinされてなければAllOneが返る
	Bitboard pinMaskGote(const unsigned pos)const;

private:
	void setKoma(size_t pos, koma::Koma k) { assert(0 <= pos && pos < 81); bammen[pos] = static_cast<std::uint8_t>(k); }
	void reflectBitboard();
	bool pinCheck(const unsigned ou,Bitboard line, const Bitboard& enemies)const;
	Bitboard pinned_linescan(const koma::Vector2 ou, const koma::Vector2 dir, const std::vector<koma::Koma> scanned)const;

	bool isSente;
	std::array<std::uint8_t,95> bammen;//81番以降は持ち駒の数 SS2ndでは別の配列にしていたのを統合
	Bitboard allKomaBB;
	Bitboard senteKomaBB;
	Bitboard goteKomaBB;
	std::array<Bitboard, static_cast<size_t>(koma::Koma::KomaNum)> eachKomaBB;

};