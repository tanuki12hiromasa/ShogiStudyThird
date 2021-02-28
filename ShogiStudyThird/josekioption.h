#pragma once
#include <string>
#include <unordered_map>

class JosekiOption {
public:
	void addOption(std::string name, std::string type, std::string value);
	void setOption(std::string line);
	void setOption(std::vector<std::string> tokens);
	void setOption(std::string name, std::string value);

	void coutOption();

	std::string getS(std::string name);
	int getI(std::string name);
	double getD(std::string name);
	bool getC(std::string name);
private:
	struct josekioption {
		std::string name;
		std::string value;
		std::string type;
		std::string defaultvalue;
		josekioption(std::string name, std::string type, std::string value);
		josekioption();
	};
	std::unordered_map<std::string,JosekiOption::josekioption> optionlist;
};
