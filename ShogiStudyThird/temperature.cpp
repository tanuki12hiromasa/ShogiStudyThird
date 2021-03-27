#include "temperature.h"

double SearchTemperature::Ts_max = 200;
double SearchTemperature::Ts_min = 40;
int SearchTemperature::TsDistFuncCode = 0;
int SearchTemperature::TsNodeFuncCode = 0;
double SearchTemperature::TsNodeFuncConstant = 1;

double SearchTemperature::Td = 90;
double SearchTemperature::Te = 40;

double SearchTemperature::getTs(const double& rel_number) {
	switch (TsDistFuncCode)
	{
		default:
		case 0:
			return (Ts_max + Ts_min) / 2;
		case 1:
			return Ts_min + (Ts_max - Ts_min) * rel_number;
		case 2:
		{
			const double minlog = std::log(Ts_min), maxlog = std::log(Ts_max);
			const double delta = (maxlog - minlog) * rel_number;
			return std::exp(minlog + delta);
		}
		case 3:
		{
			const double c = (Ts_max + Ts_min) / 10.0;
			const double a = 1.0 / (std::exp((Ts_max - Ts_min) / (c * 2.0)) - 1.0);
			return c * std::log((rel_number + a) / (1 + a - rel_number)) + (Ts_min + Ts_max) / 2.0;
		}
		case 4:
		{
			const double minlog = std::log(Ts_min), maxlog = std::log(Ts_max);
			const double c = (minlog + maxlog) / 40.0;
			const double a = 1.0 / (std::exp((maxlog - minlog) / (c * 2.0)) - 1.0);
			return std::exp(c * std::log((rel_number + a) / (1 + a - rel_number)) + (minlog + maxlog) / 2.0);
		}
	}
}

double SearchTemperature::getTs_atRoot(const double& rel_number,const SearchNode& root) {
	switch (TsDistFuncCode)
	{
		default:
		case 0:
			return (Ts_max + Ts_min) / 2;
		case 1:
			return Ts_min + (Ts_max - Ts_min) * rel_number;
		case 2:
		{
			const double minlog = std::log(Ts_min), maxlog = std::log(Ts_max);
			const double delta = (maxlog - minlog) / rel_number;
			return std::exp(minlog + delta);
		}
		case 3:
		{
			const double c = (Ts_max + Ts_min) / 10.0;
			const double a = 1.0 / (std::exp((Ts_max - Ts_min) / (c * 2.0)) - 1.0);
			return c * std::log((rel_number + a) / (1 + a - rel_number)) + (Ts_min + Ts_max) / 2.0;
		}
		case 4:
		{
			const double minlog = std::log(Ts_min), maxlog = std::log(Ts_max);
			const double c = (minlog + maxlog) / 40.0;
			const double a = 1.0 / (std::exp((maxlog - minlog) / (c * 2.0)) - 1.0);
			return std::exp(c * std::log((rel_number + a) / (1 + a - rel_number)) + (minlog + maxlog) / 2.0);
		}
	}
}

double SearchTemperature::getTs_atNode(const double& T, const SearchNode& node) {
	switch (TsNodeFuncCode)
	{
		case 0:
		default:
			return T;
		case 1:
			return T * std::pow(TsNodeFuncConstant, node.mass);
	}
}