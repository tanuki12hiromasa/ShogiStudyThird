#include "stdafx.h"
#include "time_property.h"

TimeProperty::TimeProperty(bool teban, const std::vector<std::string>& tokens):
	rule(TimeRule::byoyomi),
	left(5000),added(0),
	rival_left(5000),rival_added(0)
{
	if (tokens[1] == "infinite") {
		rule = TimeRule::infinite;
	}
	else {
		if (teban) {
			left = std::chrono::milliseconds(std::stoi(tokens[2]));
			rival_left = std::chrono::milliseconds(std::stoi(tokens[4]));
		}
		else {
			left = std::chrono::milliseconds(std::stoi(tokens[4]));
			rival_left = std::chrono::milliseconds(std::stoi(tokens[2]));
		}
		if (tokens.size() > 5) {
			if (tokens[5] == "byoyomi") {
				//ストップウォッチルールの切り捨て時間
				rule = TimeRule::byoyomi;
				added = std::chrono::milliseconds(std::stoi(tokens[6]));
				rival_added = std::chrono::milliseconds(std::stoi(tokens[6]));
			}
			else {
				//フィッシャークロックルールの追加時間
				rule = TimeRule::fischer;
				if (teban) {
					added = std::chrono::milliseconds(std::stoi(tokens[6]));
					rival_added = std::chrono::milliseconds(std::stoi(tokens[8]));
				}
				else {
					added = std::chrono::milliseconds(std::stoi(tokens[8]));
					rival_added = std::chrono::milliseconds(std::stoi(tokens[6]));
				}
			}
		}

	}
}