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
			commander.tree.set(tokens);
		}
		else if (tokens[0] == "go") {
			if (tokens[1] == "mate") {
				//詰将棋は非対応
				std::cout << "checkmate notimplemented" << std::endl;
				continue;
			}
			commander.go(tokens);
		}
		else if (tokens[0] == "stop") {
			commander.chakushu();
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

void Commander::coutOption() {

}

void Commander::setOption(std::vector<std::string>& token) {
	if (token[2] == "USI_Ponder") {
		if (token.size <= 3 || token[4] == "false") {
			permitPonder = false;
			std::cout << "ponder : false" << std::endl;
		}
		else {
			permitPonder = true;
			std::cout << "ponder : true" << std::endl;
		}
	}
}

void Commander::paramInit() {
	//filepathから設定ファイルを読み込む
	tree.setTchoice({ 10,40,90,120 });
	tree.setTchoice_q(90);
	tree.setTdepth(90);
	tree.setTeval(20);
	tree.setMassmaxInQSearch(6);
	tree.setNodeMaxsize(100000000);
	SearchNode::setMateScore(34000);
	SearchNode::setMateOneScore(20);
	SearchNode::setMateScoreBound(30000);
	SearchNode::setRepScore(-200);
	agentNum = 12;
}

void Commander::gameInit() {
	if (agents.empty()) {
		BBkiki::init();
		Evaluator::init();
	}
	else {
		for (auto& ag : agents) {
			ag.terminate();
		}
		for (auto& th : agent_threads) {
			th.join();
		}
		agents.clear();
		agent_threads.clear();
	}
	for (unsigned i = 0; i < agentNum; i++) {
		agents.emplace_back(SearchAgent(tree, i));
	}
	for (auto& ag : agents) {
		agent_threads.emplace_back(std::thread(&SearchAgent::loop, &ag));
	}
	info();
}

void Commander::go(std::vector<std::string>& tokens) {
	//宣言可能かどうかは先に調べる
	const Kyokumen& kyokumen = tree.getRootPlayer().kyokumen;
	if (kyokumen.isDeclarable()) {
		std::cout << "bestmove win" << std::endl;
		return;
	}
	tree.permitSearch();
	TimeProperty tp(kyokumen.teban(), tokens);
	go_alive = false;
	go_thread.join();
	go_thread = std::thread([this,tp]() 
		{
			
		}
	);
	info_enable = true;
}

void Commander::info() {

}

void Commander::chakushu() {
	std::lock_guard<std::mutex> lock(coutmtx);
	tree.prohibitSearch();
	info_enable = false;
	SearchNode* bestchild = tree.getBestMove();
	
}