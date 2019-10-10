#include "stdafx.h"
#include "move.h"
#include "usi.h"

std::vector<Move> Move::usiToMoves(const std::vector<std::string>& tokens) {

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