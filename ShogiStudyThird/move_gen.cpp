#include "stdafx.h"
#include "move_gen.h"

struct GeneratedMoves {
	void add(Move move) {
		moves.push_back(move);
	}
	std::vector<Move> moves;
};

inline void addByBitboard(GeneratedMoves& en, unsigned from, Bitboard toBB, bool prom) {
	for (unsigned to = toBB.pop_first(); to < toBB.size(); to = toBB.pop_first()) {
		en.add(Move(from, to, prom));
	}
}

inline void genSenteBanMove_koma(GeneratedMoves& en, const Kyokumen& kyokumen, koma::Koma piece, bool prom, const Bitboard& mask) {
	const Bitboard& allBB = kyokumen.getAllBB();
	Bitboard komabb = kyokumen.getEachBB(piece);
	for (unsigned from = komabb.pop_first(); from < komabb.size(); from = komabb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskSente(from) & mask;
		addByBitboard(en, from, kikibb, prom);
	}
}

inline void genGoteBanMove_koma(GeneratedMoves& en, const Kyokumen& kyokumen, koma::Koma piece, bool prom, const Bitboard& mask) {
	const Bitboard& allBB = kyokumen.getAllBB();
	Bitboard komabb = kyokumen.getEachBB(piece);
	for (unsigned from = komabb.pop_first(); from < komabb.size(); from = komabb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskGote(from) & mask;
		addByBitboard(en, from, kikibb, prom);
	}
}

inline void genSenteBanMove_koma(GeneratedMoves& en, const Kyokumen& kyokumen, koma::Koma piece, bool prom, Bitboard frombb, const Bitboard& tomask) {
	const Bitboard& allBB = kyokumen.getAllBB();
	for (unsigned from = frombb.pop_first(); from < frombb.size(); from = frombb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskSente(from) & tomask;
		addByBitboard(en, from, kikibb, prom);
	}
}

inline void genGoteBanMove_koma(GeneratedMoves& en, const Kyokumen& kyokumen, koma::Koma piece, bool prom, Bitboard frombb, const Bitboard& tomask) {
	const Bitboard& allBB = kyokumen.getAllBB();
	for (unsigned from = frombb.pop_first(); from < frombb.size(); from = frombb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskGote(from) & tomask;
		addByBitboard(en, from, kikibb, prom);
	}
}

inline void genSenteOuMove(GeneratedMoves& en, const Kyokumen& kyokumen, const Bitboard& mask) {
	using namespace koma;
	const unsigned oupos = kyokumen.sOuPos();
	Bitboard enemykikibb;
	Bitboard goteBB = kyokumen.getGoteBB();
	Bitboard allBB = kyokumen.getAllBB();
	allBB.reset(oupos);
	for (unsigned pos = goteBB.pop_first(); pos < goteBB.size(); pos = goteBB.pop_first()) {
		Koma k = kyokumen.getKoma(pos);
		enemykikibb |= BBkiki::getKiki(allBB, k, pos);
	}
	Bitboard kikibb = BBkiki::getStepKiki(Koma::s_Ou, oupos) & ~enemykikibb & mask;
	addByBitboard(en, oupos, kikibb, false);
}

inline void genGoteOuMove(GeneratedMoves& en, const Kyokumen& kyokumen, const Bitboard& mask) {
	using namespace koma;
	const unsigned oupos = kyokumen.gOuPos();
	Bitboard enemykikibb;
	Bitboard senteBB = kyokumen.getSenteBB();
	Bitboard allBB = kyokumen.getAllBB();
	allBB.reset(oupos);
	for (unsigned pos = senteBB.pop_first(); pos < senteBB.size(); pos = senteBB.pop_first()) {
		Koma k = kyokumen.getKoma(pos);
		enemykikibb |= BBkiki::getKiki(allBB, k, pos);
	}
	Bitboard  kikibb = BBkiki::getStepKiki(Koma::g_Ou, oupos) & ~enemykikibb & mask;
	addByBitboard(en, oupos, kikibb, false);
}

void genAllSenteBanMove(GeneratedMoves& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {
	using namespace koma;
	genSenteBanMove_koma(en, kyokumen, Koma::s_Fu, false, toMaskBB & bbmask::Dan2to9);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Fu, true, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kei, false, toMaskBB & bbmask::Dan3to9);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kei, true, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Gin, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Gin, true, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Gin, true, kyokumen.getEachBB(Koma::s_Gin) & bbmask::Dan3, toMaskBB & bbmask::Dan4to9);

	genSenteBanMove_koma(en, kyokumen, Koma::s_Kin, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nFu, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nKyou, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nKei, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nGin, false, toMaskBB);

	genSenteBanMove_koma(en, kyokumen, Koma::s_Kyou, false, toMaskBB & bbmask::Dan2to9);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kyou, true, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, false, kyokumen.getEachBB(Koma::s_Kaku), toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, true, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan4to9, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, true, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan1to3, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, false, kyokumen.getEachBB(Koma::s_Hi), toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, true, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan4to9, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, true, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan1to3, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nKaku, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nHi, false, toMaskBB);
}

void genAllGoteBanMove(GeneratedMoves& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {
	using namespace koma;
	genGoteBanMove_koma(en, kyokumen, Koma::g_Fu, false, toMaskBB & bbmask::Dan1to8);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Fu, true, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kei, false, toMaskBB & bbmask::Dan1to7);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kei, true, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Gin, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Gin, true, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Gin, true, kyokumen.getEachBB(Koma::g_Gin) & bbmask::Dan7, toMaskBB & bbmask::Dan1to6);

	genGoteBanMove_koma(en, kyokumen, Koma::g_Kin, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nFu, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nKyou, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nKei, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nGin, false, toMaskBB);

	genGoteBanMove_koma(en, kyokumen, Koma::g_Kyou, false, toMaskBB & bbmask::Dan1to8);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kyou, true, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, false, kyokumen.getEachBB(Koma::g_Kaku), toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, true, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan1to6, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, true, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan7to9, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, false, kyokumen.getEachBB(Koma::g_Hi), toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, true, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan1to6, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, true, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan7to9, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nKaku, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nHi, false, toMaskBB);
}

void genSenteBanMove(GeneratedMoves& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {//不要な不成(歩,香2段目,角,飛)を生成しない
	using namespace koma;
	genSenteBanMove_koma(en, kyokumen, Koma::s_Fu, false, kyokumen.getEachBB(Koma::s_Fu) & bbmask::Dan5to9, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Fu, true, kyokumen.getEachBB(Koma::s_Fu) & bbmask::Dan1to4, toMaskBB);//4段目から出発したら確実に成れるのでtomaskはそのまま
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kei, false, toMaskBB & bbmask::Dan3to9);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kei, true, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Gin, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Gin, true, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Gin, true, kyokumen.getEachBB(Koma::s_Gin) & bbmask::Dan3, toMaskBB & bbmask::Dan4to9);

	genSenteBanMove_koma(en, kyokumen, Koma::s_Kin, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nFu, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nKyou, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nKei, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nGin, false, toMaskBB);

	genSenteBanMove_koma(en, kyokumen, Koma::s_Kyou, false, toMaskBB & bbmask::Dan3to9);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kyou, true, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, false, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan4to9, toMaskBB & bbmask::Dan4to9);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, true, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan4to9, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, true, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan1to3, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, false, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan4to9, toMaskBB & bbmask::Dan4to9);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, true, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan4to9, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, true, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan1to3, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nKaku, false, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nHi, false, toMaskBB);
}

void genGoteBanMove(GeneratedMoves& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {
	using namespace koma;
	genGoteBanMove_koma(en, kyokumen, Koma::g_Fu, false, kyokumen.getEachBB(Koma::g_Fu) & bbmask::Dan1to5, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Fu, true, kyokumen.getEachBB(Koma::g_Fu) & bbmask::Dan6to9, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kei, false, toMaskBB & bbmask::Dan1to7);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kei, true, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Gin, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Gin, true, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Gin, true, kyokumen.getEachBB(Koma::g_Gin) & bbmask::Dan7, toMaskBB & bbmask::Dan1to6);

	genGoteBanMove_koma(en, kyokumen, Koma::g_Kin, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nFu, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nKyou, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nKei, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nGin, false, toMaskBB);

	genGoteBanMove_koma(en, kyokumen, Koma::g_Kyou, false, toMaskBB & bbmask::Dan1to7);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kyou, true, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, false, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan1to6, toMaskBB & bbmask::Dan1to6);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, true, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan1to6, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, true, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan7to9, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, false, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan1to6, toMaskBB & bbmask::Dan1to6);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, true, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan1to6, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, true, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan7to9, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nKaku, false, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nHi, false, toMaskBB);
}

void genRemovedSenteBanMove(GeneratedMoves& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {
	using namespace koma;
	genSenteBanMove_koma(en, kyokumen, Koma::s_Fu, false, kyokumen.getEachBB(Koma::s_Fu) & bbmask::Dan1to4, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kyou, false, toMaskBB & bbmask::Dan2);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, false, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan4to9, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, false, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan1to3, toMaskBB);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, false, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan4to9, toMaskBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, false, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan1to3, toMaskBB);
}

void genRemovedGoteBanMove(GeneratedMoves& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {
	using namespace koma;
	genGoteBanMove_koma(en, kyokumen, Koma::g_Fu, false, kyokumen.getEachBB(Koma::g_Fu) & bbmask::Dan6to9, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kyou, false, toMaskBB & bbmask::Dan8);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, false, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan1to6, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, false, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan7to9, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, false, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan1to6, toMaskBB & bbmask::Dan7to9);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, false, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan7to9, toMaskBB);
}

void genSenteBanMove_noMate(GeneratedMoves& en, const Kyokumen& kyokumen, Bitboard toMaskBB) {//不要な不成(歩,香2段目,角,飛),駒損する手,王手を生成しない
	using namespace koma;
	const auto& allBB = kyokumen.getAllBB();
	Bitboard goteKikiBB = kyokumen.goteKiki_ingnoreKing();
	const unsigned oupos = kyokumen.gOuPos();

	genSenteBanMove_koma(en, kyokumen, Koma::s_Fu, false, kyokumen.getEachBB(Koma::s_Fu) & bbmask::Dan5to9, toMaskBB & ~BBkiki::getStepKiki(Koma::g_Fu, oupos));
	Bitboard nomateArea = ~BBkiki::getStepKiki(Koma::g_Kin, oupos);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Fu, true, kyokumen.getEachBB(Koma::s_Fu) & bbmask::Dan1to4, toMaskBB & nomateArea);//4段目から出発したら確実に成れるのでtomaskはそのまま

	toMaskBB &= ~(kyokumen.getEachBB(Koma::g_Fu) & goteKikiBB);//ヒモつきの歩をターゲットから除外
	genSenteBanMove_koma(en, kyokumen, Koma::s_nFu, false, toMaskBB & nomateArea);

	toMaskBB &= ~(kyokumen.getEachBB(Koma::g_nFu) & goteKikiBB);//ヒモつきのと金をターゲットから除外
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kyou, false, toMaskBB & bbmask::Dan3to9 & ~BBkiki::getDashKiki(allBB,Koma::g_Kyou, oupos));
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kei, false, toMaskBB & bbmask::Dan3to9 & ~BBkiki::getStepKiki(Koma::g_Kei, oupos));
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kyou, true, toMaskBB & bbmask::Dan1to3 & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kei, true, toMaskBB & bbmask::Dan1to3 & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nKyou, false, toMaskBB & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nKei, false, toMaskBB & nomateArea);

	toMaskBB &= ~((kyokumen.getEachBB(Koma::g_Kyou) | kyokumen.getEachBB(Koma::g_Kei) | kyokumen.getEachBB(Koma::g_nKei) | kyokumen.getEachBB(Koma::g_nKyou)) & goteKikiBB);//ヒモつきの桂香成桂成香をターゲットから除外
	genSenteBanMove_koma(en, kyokumen, Koma::s_Gin, false, toMaskBB & ~BBkiki::getStepKiki(Koma::g_Gin, oupos));
	genSenteBanMove_koma(en, kyokumen, Koma::s_Gin, true, toMaskBB & bbmask::Dan1to3 & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Gin, true, kyokumen.getEachBB(Koma::s_Gin) & bbmask::Dan3, toMaskBB & bbmask::Dan4to9 & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kin, false, toMaskBB & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nGin, false, toMaskBB & nomateArea);

	toMaskBB &= ~((kyokumen.getEachBB(Koma::g_Gin) | kyokumen.getEachBB(Koma::g_Kin) | kyokumen.getEachBB(Koma::g_nGin)) & goteKikiBB);//ヒモつきの金銀成銀をターゲットから除外
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, false, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan4to9, toMaskBB & bbmask::Dan4to9 & ~BBkiki::getDashKiki(allBB, Koma::g_Hi, oupos));
	nomateArea = ~BBkiki::getDashKiki(allBB, Koma::g_nHi, oupos);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, true, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan4to9, toMaskBB & bbmask::Dan1to3 & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Hi, true, kyokumen.getEachBB(Koma::s_Hi) & bbmask::Dan1to3, toMaskBB & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, false, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan4to9, toMaskBB & bbmask::Dan4to9 & ~BBkiki::getDashKiki(allBB, Koma::g_Kaku, oupos));
	nomateArea = ~BBkiki::getDashKiki(allBB, Koma::g_nKaku, oupos);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, true, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan4to9, toMaskBB & bbmask::Dan1to3 & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_Kaku, true, kyokumen.getEachBB(Koma::s_Kaku) & bbmask::Dan1to3, toMaskBB & nomateArea);

	//toMaskBB &= ~((kyokumen.getEachBB(Koma::g_Hi) | kyokumen.getEachBB(Koma::g_Kaku)) & goteKikiBB);//ヒモつきの飛車角をターゲットから除外
	genSenteBanMove_koma(en, kyokumen, Koma::s_nKaku, false, toMaskBB & nomateArea);
	genSenteBanMove_koma(en, kyokumen, Koma::s_nHi, false, toMaskBB & ~BBkiki::getDashKiki(allBB, Koma::g_nHi, oupos));
}

void genGoteBanMove_noMate(GeneratedMoves& en, const Kyokumen& kyokumen, Bitboard toMaskBB) {
	using namespace koma;
	const auto& allBB = kyokumen.getAllBB();
	Bitboard senteKikiBB = kyokumen.senteKiki_ingnoreKing();
	const unsigned oupos = kyokumen.sOuPos();

	genGoteBanMove_koma(en, kyokumen, Koma::g_Fu, false, kyokumen.getEachBB(Koma::g_Fu) & bbmask::Dan1to5, toMaskBB & ~BBkiki::getStepKiki(Koma::s_Fu, oupos));
	Bitboard nomateArea = ~BBkiki::getStepKiki(Koma::s_Kin, oupos);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Fu, true, kyokumen.getEachBB(Koma::g_Fu) & bbmask::Dan6to9, toMaskBB & nomateArea);

	toMaskBB &= ~(kyokumen.getEachBB(Koma::s_Fu) & senteKikiBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nFu, false, toMaskBB & nomateArea);

	toMaskBB &= ~(kyokumen.getEachBB(Koma::s_nFu) & senteKikiBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kyou, false, toMaskBB & bbmask::Dan1to7 & ~BBkiki::getDashKiki(allBB, Koma::s_Kyou, oupos));
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kei, false, toMaskBB & bbmask::Dan1to7 & ~BBkiki::getStepKiki(Koma::s_Kei, oupos));
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kyou, true, toMaskBB & bbmask::Dan7to9 & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kei, true, toMaskBB & bbmask::Dan7to9 & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nKyou, false, toMaskBB & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nKei, false, toMaskBB & nomateArea);

	toMaskBB &= ~((kyokumen.getEachBB(Koma::s_Kyou) | kyokumen.getEachBB(Koma::s_Kei) | kyokumen.getEachBB(Koma::s_nKei) | kyokumen.getEachBB(Koma::s_nKyou)) & senteKikiBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Gin, false, toMaskBB & ~BBkiki::getStepKiki(Koma::s_Gin, oupos));
	genGoteBanMove_koma(en, kyokumen, Koma::g_Gin, true, toMaskBB & bbmask::Dan7to9 & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Gin, true, kyokumen.getEachBB(Koma::g_Gin) & bbmask::Dan7, toMaskBB & bbmask::Dan1to6 & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kin, false, toMaskBB & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nGin, false, toMaskBB & nomateArea);

	toMaskBB &= ~((kyokumen.getEachBB(Koma::s_Gin) | kyokumen.getEachBB(Koma::s_Kin) | kyokumen.getEachBB(Koma::s_nGin)) & senteKikiBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, false, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan1to6, toMaskBB & bbmask::Dan1to6 & ~BBkiki::getDashKiki(allBB, Koma::s_Hi, oupos));
	nomateArea = ~BBkiki::getDashKiki(allBB, Koma::s_nHi, oupos);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, true, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan1to6, toMaskBB & bbmask::Dan7to9 & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Hi, true, kyokumen.getEachBB(Koma::g_Hi) & bbmask::Dan7to9, toMaskBB & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, false, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan1to6, toMaskBB & bbmask::Dan1to6 & ~BBkiki::getDashKiki(allBB, Koma::s_Kaku, oupos));
	nomateArea = ~BBkiki::getDashKiki(allBB, Koma::s_nKaku, oupos);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, true, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan1to6, toMaskBB & bbmask::Dan7to9 & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Kaku, true, kyokumen.getEachBB(Koma::g_Kaku) & bbmask::Dan7to9, toMaskBB & nomateArea);

	//toMaskBB &= ~((kyokumen.getEachBB(Koma::s_Kaku) | kyokumen.getEachBB(Koma::s_Hi)) & senteKikiBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nKaku, false, toMaskBB & nomateArea);
	genGoteBanMove_koma(en, kyokumen, Koma::g_nHi, false, toMaskBB & ~BBkiki::getDashKiki(allBB, Koma::s_nHi, oupos));
}

void genSenteUchiMove(GeneratedMoves& en, const Kyokumen& kyokumen,const Bitboard& blanks) {
	using namespace koma;
	Bitboard invAllBB = ~kyokumen.getAllBB();
	if (kyokumen.getMochigomaNum(Position::m_sFu) > 0) {
		Bitboard UchiFuBB = kyokumen.getEachBB(Koma::s_Fu).getNoFuLines() & blanks & bbmask::Dan2to9;
		addByBitboard(en, Position::m_sFu, UchiFuBB, false);
	}
	if (kyokumen.getMochigomaNum(Position::m_sKyou) > 0) {
		addByBitboard(en, Position::m_sKyou, blanks & bbmask::Dan2to9, false);
	}
	if (kyokumen.getMochigomaNum(Position::m_sKei) > 0) {
		addByBitboard(en, Position::m_sKei, blanks & bbmask::Dan3to9, false);
	}
	for (Position mpos : {Position::m_sGin, Position::m_sKin, Position::m_sKaku, Position::m_sHi}) {
		if (kyokumen.getMochigomaNum(mpos) > 0) {
			addByBitboard(en, mpos, blanks, false);
		}
	}
}

void genGoteUchiMove(GeneratedMoves& en, const Kyokumen& kyokumen,const Bitboard& blanks) {
	using namespace koma;
	Bitboard invAllBB = ~kyokumen.getAllBB();
	if (kyokumen.getMochigomaNum(Position::m_gFu) > 0) {
		Bitboard UchiFuBB = kyokumen.getEachBB(Koma::g_Fu).getNoFuLines() & blanks & bbmask::Dan1to8;
		addByBitboard(en, Position::m_gFu, UchiFuBB, false);
	}
	if (kyokumen.getMochigomaNum(Position::m_gKyou) > 0) {
		addByBitboard(en, Position::m_gKyou, blanks & bbmask::Dan1to8, false);
	}
	if (kyokumen.getMochigomaNum(Position::m_gKei) > 0) {
		addByBitboard(en, Position::m_gKei, blanks & bbmask::Dan1to7, false);
	}
	for (Position mpos : {Position::m_gGin, Position::m_gKin, Position::m_gKaku, Position::m_gHi}) {
		if (kyokumen.getMochigomaNum(mpos) > 0) {
			addByBitboard(en, mpos, blanks, false);
		}
	}
}

std::vector<Move> MoveGenerator::genMove(Move& move, const Kyokumen& kyokumen) {
	GeneratedMoves en;
	en.moves.reserve(512);
	if (kyokumen.teban()) {
		auto kusemono = kyokumen.getSenteOuCheck(move);
		if (kusemono.size() > 0) {
			move.setOute(true);
			if (kusemono.size() == 1) {
				genSenteBanMove(en, kyokumen, kusemono[0]);
				genSenteUchiMove(en, kyokumen, kusemono[0] & ~kyokumen.getAllBB());
			}
			genSenteOuMove(en, kyokumen, ~kyokumen.getSenteBB());
		}
		else {
			genSenteBanMove(en, kyokumen, ~kyokumen.getSenteBB());
			genSenteUchiMove(en, kyokumen, ~kyokumen.getAllBB());
			genSenteOuMove(en, kyokumen, ~kyokumen.getSenteBB());
		}
	}
	else {
		auto kusemono = kyokumen.getGoteOuCheck(move);
		if (kusemono.size() > 0) {
			move.setOute(true);
			if (kusemono.size() == 1) {
				genGoteBanMove(en, kyokumen, kusemono[0]);
				genGoteUchiMove(en, kyokumen, kusemono[0] & ~kyokumen.getAllBB());
			}
			genGoteOuMove(en, kyokumen, ~kyokumen.getGoteBB());
		}
		else {
			genGoteBanMove(en, kyokumen, ~kyokumen.getGoteBB());
			genGoteUchiMove(en, kyokumen, ~kyokumen.getAllBB());
			genGoteOuMove(en, kyokumen, ~kyokumen.getGoteBB());
		}
	}
	en.moves.shrink_to_fit();
	return en.moves;
}

std::pair<std::vector<Move>, std::vector<Move>> MoveGenerator::genMoveWithRemoved(Move& move, const Kyokumen& kyokumen) {
	GeneratedMoves en, removed;
	en.moves.reserve(512);
	if (kyokumen.teban()) {
		auto kusemono = kyokumen.getSenteOuCheck(move);
		if (kusemono.size() > 0) {
			move.setOute(true);
			if (kusemono.size() == 1) {
				genSenteBanMove(en, kyokumen, kusemono[0]);
				genRemovedSenteBanMove(removed, kyokumen, kusemono[0]);
				genSenteUchiMove(en, kyokumen, kusemono[0] & ~kyokumen.getAllBB());
			}
			genSenteOuMove(en, kyokumen, ~kyokumen.getSenteBB());
		}
		else {
			genSenteBanMove(en, kyokumen, ~kyokumen.getSenteBB());
			genRemovedSenteBanMove(removed, kyokumen, ~kyokumen.getSenteBB());
			genSenteUchiMove(en, kyokumen, ~kyokumen.getAllBB());
			genSenteOuMove(en, kyokumen, ~kyokumen.getSenteBB());
		}
	}
	else {
		auto kusemono = kyokumen.getGoteOuCheck(move);
		if (kusemono.size() > 0) {
			move.setOute(true);
			if (kusemono.size() == 1) {
				genGoteBanMove(en, kyokumen, kusemono[0]);
				genRemovedGoteBanMove(removed, kyokumen, kusemono[0]);
				genGoteUchiMove(en, kyokumen, kusemono[0] & ~kyokumen.getAllBB());
			}
			genGoteOuMove(en, kyokumen, ~kyokumen.getGoteBB());
		}
		else {
			genGoteBanMove(en, kyokumen, ~kyokumen.getGoteBB());
			genRemovedGoteBanMove(removed, kyokumen, ~kyokumen.getGoteBB());
			genGoteUchiMove(en, kyokumen, ~kyokumen.getAllBB());
			genGoteOuMove(en, kyokumen, ~kyokumen.getGoteBB());
		}
	}
	en.moves.shrink_to_fit();
	removed.moves.shrink_to_fit();
	return std::make_pair(std::move(en.moves), std::move(removed.moves));
}

std::vector<Move> MoveGenerator::genCapMove(Move& move, const Kyokumen& kyokumen) {
	GeneratedMoves en;
	if (kyokumen.teban()) {
		auto kusemono = kyokumen.getSenteOuCheck(move);
		if (kusemono.size() > 0) {
			move.setOute(true);
			if (kusemono.size() == 1) {
				genSenteBanMove(en, kyokumen, kusemono[0]);
				genSenteUchiMove(en, kyokumen, kusemono[0] & ~kyokumen.getAllBB());
			}
			genSenteOuMove(en, kyokumen, ~kyokumen.getSenteBB());
		}
		else {
			genSenteBanMove_noMate(en, kyokumen, kyokumen.getGoteBB());
			genSenteOuMove(en, kyokumen, kyokumen.getGoteBB());
		}
	}
	else {
		auto kusemono = kyokumen.getGoteOuCheck(move);
		if (kusemono.size() > 0) {
			move.setOute(true);
			if (kusemono.size() == 1) {
				genGoteBanMove(en, kyokumen, kusemono[0]);
				genGoteUchiMove(en, kyokumen, kusemono[0] & ~kyokumen.getAllBB());
			}
			genGoteOuMove(en, kyokumen, ~kyokumen.getGoteBB());
		}
		else {
			genGoteBanMove_noMate(en, kyokumen, kyokumen.getSenteBB());
			genGoteOuMove(en, kyokumen, kyokumen.getSenteBB());
		}
	}
	en.moves.shrink_to_fit();
	return en.moves;
}

//もともとはcapMoveの補集合だったが、現在はcapmoveで刈った駒損する手や王手になる手が保管されていないので注意（test上でエラーになる）
std::vector<Move> MoveGenerator::genNocapMove(Move& move, const Kyokumen& kyokumen) {
	GeneratedMoves en;
	if (kyokumen.teban()) {
		auto kusemono = kyokumen.getSenteOuCheck(move);
		if (kusemono.size() == 0){
			genSenteBanMove(en, kyokumen, ~kyokumen.getAllBB());
			genSenteUchiMove(en, kyokumen, ~kyokumen.getAllBB());
			genSenteOuMove(en, kyokumen, ~kyokumen.getAllBB());
		}
		else {
			move.setOute(true);
		}
	}
	else {
		auto kusemono = kyokumen.getGoteOuCheck(move);
		if (kusemono.size() == 0) {
			genGoteBanMove(en, kyokumen, ~kyokumen.getAllBB());
			genGoteUchiMove(en, kyokumen, ~kyokumen.getAllBB());
			genGoteOuMove(en, kyokumen, ~kyokumen.getAllBB());
		}
		else {
			move.setOute(true);
		}
	}
	return en.moves;
}

std::vector<Move> MoveGenerator::genAllMove(Move& move, const Kyokumen& kyokumen) {
	GeneratedMoves en;
	en.moves.reserve(512);
	if (kyokumen.teban()) {
		auto kusemono = kyokumen.getSenteOuCheck(move);
		if (kusemono.size() > 0) {
			move.setOute(true);
			if (kusemono.size() == 1) {
				genAllSenteBanMove(en, kyokumen, kusemono[0]);
				genSenteUchiMove(en, kyokumen, kusemono[0] & ~kyokumen.getAllBB());
			}
			genSenteOuMove(en, kyokumen, ~kyokumen.getSenteBB());
		}
		else {
			genAllSenteBanMove(en, kyokumen, ~kyokumen.getSenteBB());
			genSenteUchiMove(en, kyokumen, ~kyokumen.getAllBB());
			genSenteOuMove(en, kyokumen, ~kyokumen.getSenteBB());
		}
	}
	else {
		auto kusemono = kyokumen.getGoteOuCheck(move);
		if (kusemono.size() > 0) {
			move.setOute(true);
			if (kusemono.size() == 1) {
				genAllGoteBanMove(en, kyokumen, kusemono[0]);
				genGoteUchiMove(en, kyokumen, kusemono[0] & ~kyokumen.getAllBB());
			}
			genGoteOuMove(en, kyokumen, ~kyokumen.getGoteBB());
		}
		else {
			genAllGoteBanMove(en, kyokumen, ~kyokumen.getGoteBB());
			genGoteUchiMove(en, kyokumen, ~kyokumen.getAllBB());
			genGoteOuMove(en, kyokumen, ~kyokumen.getGoteBB());
		}
	}
	return en.moves;
}