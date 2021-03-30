#pragma once
#include <cstdint>
#include "node.h"

class SearchTemperature {
public:
	static double getTs(const double& rel_number);
	static double getTs_atRoot(const double& rel_number,const SearchNode& root);
	static double getTs_atNode(const double& T,const SearchNode& node);

	static double Ts_max;
	static double Ts_min;
	static int TsDistFuncCode;
	static double TsNodeFuncConstant;
	static int TsNodeFuncCode;

	static double Td;
	static double Te;


};