#include "stdafx.h"
#include "move_gen.h"

void genEvationMove(EvaluatedNodes& en,std::vector<Bitboard>& kusemono) {

}

void MoveGenerator::genMove(EvaluatedNodes& en) {
	const Kyokumen& kyokumen = en.kyokumen;
	auto kusemono = kyokumen.getSenteOuCheck(en.parentNode->move);

}