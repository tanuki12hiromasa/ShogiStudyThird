#include "josekioption.h"
#include "usi.h"
#include <iostream>

JosekiOption::josekioption::josekioption(std::string name, std::string type, std::string value) {
	this->name = name;
	this->type = type;
	this->defaultvalue = value;
	this->value = value;
}

JosekiOption::josekioption::josekioption(){
	this->name = "null";
	this->type = "null";
	this->defaultvalue = "null";
	this->value = "null";
}

void JosekiOption::addOption(std::string name, std::string type, std::string value){
	josekioption jo(name, type, value);
	optionlist.emplace(name, jo);
}

void JosekiOption::setOption(std::string line)
{
	setOption(usi::split(line, ' '));
}

void JosekiOption::setOption(std::vector<std::string> tokens)
{
	setOption(tokens[2], tokens[4]);
}

void JosekiOption::setOption(std::string name, std::string value)
{
	if (optionlist.find(name) != optionlist.end()) {
		optionlist[name].value = value;
	}
}

void JosekiOption::coutOption()
{
	for (auto i = optionlist.begin(); i != optionlist.end(); ++i) {
		std::string name = i->second.name;
		std::string type = i->second.type;
		std::string defaultvalue = i->second.defaultvalue;

		std::cout << 
			"option name " << name << 
			" type " << type << 
			" default " << defaultvalue
			<< std::endl;
	}
}

std::string JosekiOption::getS(std::string name)
{
	return optionlist[name].value;
}

int JosekiOption::getI(std::string name)
{
	return std::stoi(optionlist[name].value);
}

double JosekiOption::getD(std::string name)
{
	return std::stod(optionlist[name].value);
}

bool JosekiOption::getC(std::string name)
{
	return optionlist[name].value == "true";
}

