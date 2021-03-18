#include "joseki.h"
#include "usi.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <Windows.h>
#include <climits>
#include <random>

HANDLE shareHandle;

Joseki::Joseki(){
	option.addOption("joseki_on", "check", "false");
	option.addOption("joseki_folder_name", "string", "joseki");
}

void Joseki::setOption(std::vector<std::string> tokens){
	option.setOption(tokens);
	input.option.setOption(tokens);
	output.option.setOption(tokens);
	josekiDataBase.option.setOption(tokens);
}
void Joseki::coutOption() {
	option.coutOption();
	input.option.coutOption();
	output.option.coutOption();
	josekiDataBase.option.coutOption();
}
