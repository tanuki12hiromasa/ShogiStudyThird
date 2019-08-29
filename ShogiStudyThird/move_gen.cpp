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

void genSenteOuMove(EvaluatedNodes& en, const Bitboard& mask) {
	using namespace koma;
	Bitboard enemykikibb;
	const Kyokumen& kyokumen = en.kyokumen;
	Bitboard goteBB = kyokumen.getGoteBB();
	const Bitboard& allBB = kyokumen.getAllBB();
	for (unsigned pos = goteBB.pop_first(); pos < goteBB.size(); pos = goteBB.pop_first()) {
		Koma k = kyokumen.getKoma(pos);
		enemykikibb |= BBkiki::getKiki(allBB, k, pos);
	}
	unsigned oupos = kyokumen.sOuPos();
	Bitboard kikibb = BBkiki::getStepKiki(Koma::s_Ou, oupos) & ~enemykikibb & mask;
	addByBitboard(en, oupos, kikibb, false);
}

void genSenteBanMove(EvaluatedNodes& en) {
	using namespace koma;
	Bitboard invSenteBB = ~en.kyokumen.getSenteBB();
	genSenteBanMove_koma(en, Koma::s_Fu, false, invSenteBB & bbmask::Dan2to9);
	genSenteBanMove_koma(en, Koma::s_Fu, true, invSenteBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_Kei, false, invSenteBB & bbmask::Dan3to9);
	genSenteBanMove_koma(en, Koma::s_Kei, true, invSenteBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_Gin, false, invSenteBB);
	genSenteBanMove_koma(en, Koma::s_Gin, true, invSenteBB & bbmask::Dan1to3);
	
	genSenteBanMove_koma(en, Koma::s_Kin, false, invSenteBB);
	genSenteBanMove_koma(en, Koma::s_nFu, false, invSenteBB);
	genSenteBanMove_koma(en, Koma::s_nKyou, false, invSenteBB);
	genSenteBanMove_koma(en, Koma::s_nKei, false, invSenteBB);
	genSenteBanMove_koma(en, Koma::s_nGin, false, invSenteBB);
	
	genSenteBanMove_koma(en, Koma::s_Kyou, false, invSenteBB & bbmask::Dan2to9);
	genSenteBanMove_koma(en, Koma::s_Kyou, true, invSenteBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_Kaku, false, invSenteBB & bbmask::AllOne);
	genSenteBanMove_koma(en, Koma::s_Kaku, true, invSenteBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_Hi, false, invSenteBB);
	genSenteBanMove_koma(en, Koma::s_Hi, true, invSenteBB & bbmask::Dan1to3);
	genSenteBanMove_koma(en, Koma::s_nKaku, false, invSenteBB);
	genSenteBanMove_koma(en, Koma::s_nHi, false, invSenteBB);

	genSenteOuMove(en, invSenteBB);
}

void genGoteBanMove(EvaluatedNodes& en) {

}

void genSenteUchiMove(EvaluatedNodes& en) {
	using namespace koma;
	Bitboard invAllBB = ~en.kyokumen.getAllBB();
	const Kyokumen& kyokumen = en.kyokumen;
	if (kyokumen.getMochigomaNum(Position::m_sFu) > 0) {
		Bitboard UchiFuBB = kyokumen.getEachBB(Koma::s_Fu).getNoFuLines() & invAllBB & bbmask::Dan2to9;
		addByBitboard(en, Position::m_sFu, UchiFuBB, false);
	}
	if (kyokumen.getMochigomaNum(Position::m_sKyou) > 0) {
		addByBitboard(en, Position::m_sKyou, invAllBB & bbmask::Dan2to9, false);
	}
	if (kyokumen.getMochigomaNum(Position::m_sKei) > 0) {
		addByBitboard(en, Position::m_sKei, invAllBB & bbmask::Dan3to9, false);
	}
	for (Position mpos : {Position::m_sGin, Position::m_sKin, Position::m_sKaku, Position::m_sHi}) {
		if (kyokumen.getMochigomaNum(mpos) > 0) {
			addByBitboard(en, mpos, invAllBB, false);
		}
	}
}

void genGoteUchiMove(EvaluatedNodes& en) {

}

void genSenteEvationMove(EvaluatedNodes& en,std::vector<Bitboard>& kusemono) {
	using namespace koma;
	if (kusemono.size() == 1) {
		//合駒の移動先
		const Bitboard& target = kusemono[0];
		//盤上の駒で合駒
		genSenteBanMove_koma(en, Koma::s_Fu, false, target & bbmask::Dan2to9);
		genSenteBanMove_koma(en, Koma::s_Fu, true, target & bbmask::Dan1to3);
		genSenteBanMove_koma(en, Koma::s_Kei, false, target & bbmask::Dan3to9);
		genSenteBanMove_koma(en, Koma::s_Kei, true, target & bbmask::Dan1to3);
		genSenteBanMove_koma(en, Koma::s_Gin, false, target);
		genSenteBanMove_koma(en, Koma::s_Gin, true, target & bbmask::Dan1to3);
		genSenteBanMove_koma(en, Koma::s_Kin, false, target);
		genSenteBanMove_koma(en, Koma::s_nFu, false, target);
		genSenteBanMove_koma(en, Koma::s_nKyou, false, target);
		genSenteBanMove_koma(en, Koma::s_nKei, false, target);
		genSenteBanMove_koma(en, Koma::s_nGin, false, target);

		genSenteBanMove_koma(en, Koma::s_Kyou, false, target & bbmask::Dan2to9);
		genSenteBanMove_koma(en, Koma::s_Kyou, true, target & bbmask::Dan1to3);
		genSenteBanMove_koma(en, Koma::s_Kaku, false, target);
		genSenteBanMove_koma(en, Koma::s_Kaku, true, target & bbmask::Dan1to3);
		genSenteBanMove_koma(en, Koma::s_Hi, false, target);
		genSenteBanMove_koma(en, Koma::s_Hi, true, target & bbmask::Dan1to3);
		genSenteBanMove_koma(en, Koma::s_nKaku, false, target);
		genSenteBanMove_koma(en, Koma::s_nHi, false, target);

		//持ち駒で合駒(開き王手の場合のみ)
		if (target.popcount() > 1) {
			const Kyokumen& kyokumen = en.kyokumen;
			if (kyokumen.getMochigomaNum(Position::m_sFu) > 0) {
				Bitboard UchiFuBB = kyokumen.getEachBB(Koma::s_Fu).getNoFuLines() & target & bbmask::Dan2to9;
				addByBitboard(en, Position::m_sFu, UchiFuBB, false);
			}
			if (kyokumen.getMochigomaNum(Position::m_sKyou) > 0) {
				addByBitboard(en, Position::m_sKyou, target & bbmask::Dan2to9, false);
			}
			if (kyokumen.getMochigomaNum(Position::m_sKei) > 0) {
				addByBitboard(en, Position::m_sKei, target & bbmask::Dan3to9, false);
			}
			for (Position mpos : {Position::m_sGin, Position::m_sKin, Position::m_sKaku, Position::m_sHi}) {
				if (kyokumen.getMochigomaNum(mpos) > 0) {
					addByBitboard(en, mpos, target, false);
				}
			}
		}
	}
	genSenteOuMove(en, ~en.kyokumen.getSenteBB());
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