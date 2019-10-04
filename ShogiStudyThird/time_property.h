#pragma once
#include <string>
#include <vector>
#include <chrono>

struct TimeProperty {
	enum class TimeRule { byoyomi, fischer, infinite };

	TimeProperty(bool teban, const std::vector<std::string>& token);
	TimeRule rule;
	std::chrono::milliseconds left;
	std::chrono::milliseconds added;
	std::chrono::milliseconds rival_left;
	std::chrono::milliseconds rival_added;
};