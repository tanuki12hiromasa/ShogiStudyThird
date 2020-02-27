#include "stdafx.h"
#include "move_gen.h"

struct EvaluatedParent {
	EvaluatedParent(SearchNode* const parent):parent(parent){}
	void add(Move move) {
		parent->addChild(move);
	}
	SearchNode* const parent;
};
struct GeneratedMoves {
	void add(Move move) {
		moves.push_back(move);
	}
	std::vector<Move> moves;
};

template<class EvaluatedNodes>
inline void addByBitboard(EvaluatedNodes& en, unsigned from, Bitboard toBB, bool prom) {
	for (unsigned to = toBB.pop_first(); to < toBB.size(); to = toBB.pop_first()) {
		en.add(Move(from, to, prom));
	}
}

template<class EvaluatedNodes>
inline void genSenteBanMove_koma(EvaluatedNodes& en, const Kyokumen& kyokumen, koma::Koma piece, bool prom, const Bitboard& mask) {
	const Bitboard& allBB = kyokumen.getAllBB();
	Bitboard komabb = kyokumen.getEachBB(piece);
	for (unsigned from = komabb.pop_first(); from < komabb.size(); from = komabb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskSente(from) & mask;
		addByBitboard(en, from, kikibb, prom);
	}
}

template<class EvaluatedNodes>
inline void genGoteBanMove_koma(EvaluatedNodes& en, const Kyokumen& kyokumen, koma::Koma piece, bool prom, const Bitboard& mask) {
	const Bitboard& allBB = kyokumen.getAllBB();
	Bitboard komabb = kyokumen.getEachBB(piece);
	for (unsigned from = komabb.pop_first(); from < komabb.size(); from = komabb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskGote(from) & mask;
		addByBitboard(en, from, kikibb, prom);
	}
}

template<class EvaluatedNodes>
inline void genSenteBanMove_koma(EvaluatedNodes& en, const Kyokumen& kyokumen, koma::Koma piece, bool prom, Bitboard frombb, const Bitboard& tomask) {
	const Bitboard& allBB = kyokumen.getAllBB();
	for (unsigned from = frombb.pop_first(); from < frombb.size(); from = frombb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskSente(from) & tomask;
		addByBitboard(en, from, kikibb, prom);
	}
}
template<class EvaluatedNodes>
inline void genGoteBanMove_koma(EvaluatedNodes& en, const Kyokumen& kyokumen, koma::Koma piece, bool prom, Bitboard frombb, const Bitboard& tomask) {
	const Bitboard& allBB = kyokumen.getAllBB();
	for (unsigned from = frombb.pop_first(); from < frombb.size(); from = frombb.pop_first()) {
		Bitboard kikibb = BBkiki::getKiki(allBB, piece, from) & kyokumen.pinMaskGote(from) & tomask;
		addByBitboard(en, from, kikibb, prom);
	}
}

template<class EvaluatedNodes>
inline void genSenteOuMove(EvaluatedNodes& en, const Kyokumen& kyokumen, const Bitboard& mask) {
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

template<class EvaluatedNodes>
inline void genGoteOuMove(EvaluatedNodes& en, const Kyokumen& kyokumen, const Bitboard& mask) {
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

template<class EvaluatedNodes>
void genAllSenteBanMove(EvaluatedNodes& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {
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

template<class EvaluatedNodes>
void genAllGoteBanMove(EvaluatedNodes& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {
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

template<class EvaluatedNodes>
void genSenteBanMove(EvaluatedNodes& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {//不要な不成(歩,香2段目,角,飛)を生成しない
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

template<class EvaluatedNodes>
void genGoteBanMove(EvaluatedNodes& en, const Kyokumen& kyokumen, const Bitboard& toMaskBB) {
	using namespace koma;
	genGoteBanMove_koma(en, kyokumen, Koma::g_Fu, false, kyokumen.getEachBB(Koma::g_Fu) & bbmask::Dan1to5, toMaskBB);
	genGoteBanMove_koma(en, kyokumen, Koma::g_Fu, true, kyokumen.getEachBB(Koma::g_Fu) & bbmask::Dan6to9, toMaskBB);
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

template<class EvaluatedNodes>
void genSenteUchiMove(EvaluatedNodes& en, const Kyokumen& kyokumen,const Bitboard& blanks) {
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

template<class EvaluatedNodes>
void genGoteUchiMove(EvaluatedNodes& en, const Kyokumen& kyokumen,const Bitboard& blanks) {
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

void MoveGenerator::genMove(SearchNode* parent, const Kyokumen& kyokumen) {
	EvaluatedParent en(parent);
	if (kyokumen.teban()) {
		auto kusemono = kyokumen.getSenteOuCheck(parent->move);
		if (kusemono.size() > 0) {
			parent->move.setOute(true);
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
		auto kusemono = kyokumen.getGoteOuCheck(parent->move);
		if (kusemono.size() > 0) {
			parent->move.setOute(true);
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
	return;
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
			genSenteBanMove(en, kyokumen, kyokumen.getGoteBB());
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
			genGoteBanMove(en, kyokumen, kyokumen.getSenteBB());
			genGoteOuMove(en, kyokumen, kyokumen.getSenteBB());
		}
	}
	return en.moves;
}

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

void MoveGenerator::genAllMove(SearchNode* parent, const Kyokumen& kyokumen) {
	EvaluatedParent en(parent);
	if (kyokumen.teban()) {
		auto kusemono = kyokumen.getSenteOuCheck(parent->move);
		if (kusemono.size() > 0) {
			parent->move.setOute(true);
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
		auto kusemono = kyokumen.getGoteOuCheck(parent->move);
		if (kusemono.size() > 0) {
			parent->move.setOute(true);
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
	return;
}