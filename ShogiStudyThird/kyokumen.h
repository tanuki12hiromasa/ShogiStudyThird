#pragma once
#include "koma.h"
#include "bitboard.h"
#include "move.h"
#include "vec2.h"

class Kyokumen {
	friend class MoveGenerator;
	friend class ShogiTest;
public:
	Kyokumen();
	Kyokumen(const std::array<koma::Koma, 81>&, const std::array<unsigned,7>&, const std::array<unsigned, 7>&, bool);
	Kyokumen(const std::array<std::uint8_t, 95>&, bool);
	Kyokumen(std::array<std::uint8_t, 95>&&, bool);
	Kyokumen(const std::vector<std::string>& usitokens);
	std::string toSfen()const;
	std::string toBanFigure()const;
	
	koma::Koma proceed(const Move);//返り値は取られた駒(captured)
	koma::Koma recede(const Move m, const koma::Koma cap);//返り値は動かした駒(to=>fromに動いた駒)

	const std::array<std::uint8_t, 95>& getBammen()const { return bammen; }
	std::uint64_t getHash()const;
	bool teban()const { return isSente; }
	koma::Koma getKoma(const koma::Position pos)const { assert(pos < 81); return static_cast<koma::Koma> (bammen[static_cast<size_t>(pos)]); }
	koma::Koma getKoma(const unsigned pos)const { assert(pos < 81); return static_cast<koma::Koma> (bammen[static_cast<size_t>(pos)]); }
	unsigned getMochigomaNum(koma::Position mpos)const { assert(mpos >= 81); return static_cast<unsigned> (bammen[static_cast<size_t>(mpos)]); }
	unsigned getMochigomaNum(bool teban, koma::Mochigoma koma)const { return teban ? static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 81]) : static_cast<unsigned>(bammen[static_cast<size_t>(koma) + 81 + 7]); }
	const Bitboard& getAllBB()const { return allKomaBB; }
	const Bitboard& getSenteBB()const { return senteKomaBB; }
	const Bitboard& getGoteBB()const { return goteKomaBB; }
	const Bitboard& getEachBB(const koma::Koma k)const { return eachKomaBB[static_cast<size_t>(k)]; }
	unsigned sOuPos()const { return eachKomaBB[static_cast<size_t>(koma::Koma::s_Ou)].find_first(); }
	unsigned gOuPos()const { return eachKomaBB[static_cast<size_t>(koma::Koma::g_Ou)].find_first(); }
	
	bool isDeclarable()const;

	std::vector<Bitboard> getSenteOuCheck(const Move)const;//moveは直前のもの
	std::vector<Bitboard> getGoteOuCheck(const Move)const;
	std::vector<Bitboard> getSenteOuCheck()const;//盤全体をチェック
	std::vector<Bitboard> getGoteOuCheck()const;

	Bitboard pinMaskSente(const unsigned pos)const;//pos上の駒が敵駒にpinされていれば移動可能な直線範囲を返す pinされてなければAllOneが返る
	Bitboard pinMaskGote(const unsigned pos)const;
	Bitboard senteKiki_ingnoreKing()const;
	Bitboard goteKiki_ingnoreKing()const;

	bool operator==(const Kyokumen& rhs)const;
	bool operator!=(const Kyokumen& rhs)const { return !operator==(rhs); }
private:
	void setKoma(size_t pos, koma::Koma k) { assert(0 <= pos && pos < 81); bammen[pos] = static_cast<std::uint8_t>(k); }
	void reflectBitboard();

	bool isSente;
	std::array<std::uint8_t,95> bammen;//81番以降は持ち駒の数 SS2ndでは別の配列にしていたのを統合
	Bitboard allKomaBB;
	Bitboard senteKomaBB;
	Bitboard goteKomaBB;
	std::array<Bitboard, static_cast<size_t>(koma::Koma::KomaNum)> eachKomaBB;

};