#include "stdafx.h"
#include "move.h"
#include "usi.h"

std::vector<Move> Move::usiToMoves(const std::vector<std::string>& tokens) {
	std::vector<Move> moves;
	auto token = tokens.begin();
	auto tend = tokens.end();
	assert(*token == "position");
	token++;
	if (*token == "startpos") {
		token += 1;
	}
	else {
		assert(*token == "sfen");
		token += 5;
	}
	if (token != tend) {
		++token;
		for (bool sengo = true; token != tend; ++token) {
			moves.emplace_back(Move(*token, sengo));
			sengo = !sengo;
		}
	}
	return moves;
}

std::string Move::toUSI() const {
	using namespace koma;
	using namespace usi;
	if ((from() == SQNum) && (to() == SQNum))return "startpos";
	std::string str;
	if (koma::isInside(from())) {
		str += posToUsi(from());
	}
	else {
		str += mochigomaToUsi(true, MposToMochi(from()));
		str += '*';
	}
	str += posToUsi(to());
	if (promote())str += '+';
	return str;
}