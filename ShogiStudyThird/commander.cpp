#include "stdafx.h"
#include "commander.h"
#include "usi.h" 
#include <iostream>

void Commander::execute() {
	Commander commander;
	while (true) {
		std::string usiin;
		std::getline(std::cin, usiin);

	}
}

Commander::Commander():
	tree(),permitPonder(false),
	filepath("./setting.txt")
{
	go_alive = false;
	info_enable = false;
	info_alive = false;
}

Commander::~Commander() {
	tree.prohibitSearch();
	go_alive = false;
	info_enable = false;
	info_alive = false;
	for (auto& ag : agents) {
		ag.terminate();
	}
	go_thread.join();
	info_thread.join();
	for (auto& th : agent_threads) {
		th.join();
	}
}

void Commander::paramInit() {
	//filepathから設定ファイルを読み込む

}

void Commander::gameInit() {

}

void Commander::go() {

}

void Commander::bestmove() {

}

void Commander::info() {

}