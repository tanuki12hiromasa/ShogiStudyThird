#pragma once
#include "evaluated_nodes.h"

class MoveGenerator {
public:
	void genMove(EvaluatedNodes&);
	void genCapMove(EvaluatedNodes&);
	void genNocapMove(EvaluatedNodes&);
};