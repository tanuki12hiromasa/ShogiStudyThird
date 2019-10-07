#include "stdafx.h"
#include "commander.h"
#include "usi.h" 
#include <iostream>

void Commander::execute() {
	Commander commander;
	while (true) {
		std::string usiin;
		std::getline(std::cin, usiin);
		auto tokens = usi::split(usiin, ' ');
		if (tokens[0] == "usi") {
#ifdef _DEBUG
			std::cout << "id name ShibauraSoftmaxThird_debug" << std::endl;
#else
			std::cout << "id name ShibauraSoftmaxThird" << std::endl;
#endif
			std::cout << "id author Iwamoto" << std::endl;
			coutOption();
			std::cout << "usiok" << std::endl;
		}
		else if (tokens[0] == "setoption") {
			commander.setOption(tokens);
			commander.paramInit();
		}
		else if (tokens[0] == "isready") {
			commander.gameInit();
			std::cout << "readyok" << std::endl;
		}
		else if (tokens[0] == "usinewgame") {
			commander.go_alive = false;
		}
		else if (tokens[0] == "position") {
			commander.go_alive = false;
			commander.tree.prohibitSearch();
			std::vector<Move> moves = Move::usiToMoves(tokens);
			commander.tree.set(Kyokumen(tokens), moves);
		}
		else if (tokens[0] == "go") {
			if (tokens[1] == "mate") {
				//詰将棋は非対応
				std::cout << "checkmate notimplemented" << std::endl;
				continue;
			}
			commander.go();
		}
		else if (tokens[0] == "stop") {
			commander.sasu();
		}
		else if (tokens[0] == "ponderhit") {
			//先読みはするがponder機能は利用しない
		}
		else if (tokens[0] == "gameover") {
			commander.go_alive = false;
			commander.info_alive = false;
			commander.tree.prohibitSearch();
		}
		else if (tokens[0] == "quit") {
			return;
		}
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