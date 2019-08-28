#include "stdafx.h"
#include "move_gen.h"
#include "bb_kiki.h"

inline void addByBitboard(EvaluatedNodes& en, unsigned from, Bitboard toBB, bool prom) {
	for (unsigned to = toBB.pop_first(); to < toBB.size(); to = toBB.pop_first()) {
		en.add(Move(from, to, prom));
	}
}

void genSenteBanMove_koma(EvaluatedNodes& en, koma::Koma piece, bool prom, const Bitboard& mask) {
	const Kyokumen& kyokumen = en.kyokumen;
	const Bitboard& allBB = kyokumen.getAllBB();
	Bitboard komabb = kyokumen.getEachBB(piece);
	for (unsigned from = komabb.pop_first(); from < komabb.size(); from = komabb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskSente(from) & mask;
		addByBitboard(en, from, kikibb, prom);
	}
}
void genGoteBanMove_koma(EvaluatedNodes& en, koma::Koma piece, bool prom, const Bitboard& mask) {
	const Kyokumen& kyokumen = en.kyokumen;
	const Bitboard& allBB = kyokumen.getAllBB();
	Bitboard komabb = kyokumen.getEachBB(piece);
	for (unsigned from = komabb.pop_first(); from < komabb.size(); from = komabb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskGote(from) & mask;
		addByBitboard(en, from, kikibb, prom);
	}
}

void genSenteOuMove(EvaluatedNodes& en) {

}

void genSenteBanMove(EvaluatedNodes& en) {
	using namespace koma;
	genSenteBanMove_koma(en, Koma::s_Fu, false, bbmask::Dan2to9);
	genSenteBanMove_koma(en, Koma::s_Fu, true, bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_Kei, false, bbmask::Dan3to9);
	genSenteBanMove_koma(en, Koma::s_Kei, true, bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_Gin, false, bbmask::AllOne);
	genSenteBanMove_koma(en, Koma::s_Gin, true, bbmask::Dan1to3);
	
	genSenteBanMove_koma(en, Koma::s_Kin, false, bbmask::AllOne);
	genSenteBanMove_koma(en, Koma::s_nFu, false, bbmask::AllOne);
	genSenteBanMove_koma(en, Koma::s_nKyou, false, bbmask::AllOne);
	genSenteBanMove_koma(en, Koma::s_nKei, false, bbmask::AllOne);
	genSenteBanMove_koma(en, Koma::s_nGin, false, bbmask::AllOne);
	
	genSenteBanMove_koma(en, Koma::s_Kyou, false, bbmask::Dan2to9);
	genSenteBanMove_koma(en, Koma::s_Kyou, true, bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_Kaku, false, bbmask::AllOne);
	genSenteBanMove_koma(en, Koma::s_Kaku, true, bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_Hi, false, bbmask::AllOne);
	genSenteBanMove_koma(en, Koma::s_Hi, true, bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_nKaku, false, bbmask::AllOne);
	genSenteBanMove_koma(en, Koma::s_nHi, false, bbmask::AllOne);

	genSenteOuMove(en);
}

void genGoteBanMove(EvaluatedNodes& en) {

}

void genSenteUchiMove(EvaluatedNodes& en) {

}

void genGoteUchiMove(EvaluatedNodes& en) {

}

void genSenteEvationMove(EvaluatedNodes& en,std::vector<Bitboard>& kusemono) {

}

void genGoteEvationMove(EvaluatedNodes& en, std::vector<Bitboard>& kusemono) {

}

void MoveGenerator::genMove(EvaluatedNodes& en) {
	const Kyokumen& kyokumen = en.kyokumen;
	
	if (kyokumen.teban()) {
		auto kusemono = kyokumen.getSenteOuCheck(en.parentNode->move);
		if (kusemono.size() > 0) {
			en.parentNode->move.setOute(true);
			genSenteEvationMove(en, kusemono);
		}
		else {
			genSenteBanMove(en);
			genSenteUchiMove(en);
		}
	}
	else {
		auto kusemono = kyokumen.getGoteOuCheck(en.parentNode->move);

	}
}