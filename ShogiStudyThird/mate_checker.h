#pragma once
#include "kyokumen.h"
#include "node.h"

class MateChecker {
public:
	void children(Kyokumen parentKyokumen, std::vector<SearchNode*> children);
};