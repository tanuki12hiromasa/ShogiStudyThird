#include "stdafx.h"
#include "move.h"
#include "usi.h"

std::string Move::toUSI() const {
	using namespace koma;
	using namespace usi;
	if (from() == SQNum && to() = SQNum)return "startpos";
	std::string str;
	if (koma::isInside(from())) {
		str += posToUsi(from());
	}
	else {
		usi += mochigomaToUsi(true, MposToMochi(from()));
		usi += '*';
	}
	usi += posToUsi(to());
	if (promote())usi += '+';
	return usi;
}