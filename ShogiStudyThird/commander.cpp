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
			bool saseta = false;
			while (!saseta) {
				saseta = commander.chakushu();
				if (!saseta)std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
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
	if(go_thread.joinable()) go_thread.join();
	if(info_thread.joinable())info_thread.join();
	for (auto& th : agent_threads) {
		if(th.joinable())th.join();
	}
}

void Commander::coutOption() {
	std::cout << "option name UseBook type check default true" << std::endl;
}

void Commander::setOption(std::vector<std::string>& token) {
	if (token[2] == "USI_Ponder") {
		if (token.size() <= 3 || token[4] == "false") {
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
		tree.rootPlayer.feature.set(tree.rootPlayer.kyokumen);
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
	tree.thread_latestRootFlags.assign(agentNum, false);
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
	if(go_thread.joinable()) go_thread.join();
	go_alive = true;
	go_thread = std::thread([this,tp]() {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(4.5s);
		while (go_alive) {
			bool saseta = chakushu();
			if (saseta) return;
			std::this_thread::sleep_for(100ms);
		}
	});
	info_enable = true;
}

void Commander::info() {
	if (!info_alive || !info_thread.joinable()) {
		if(info_thread.joinable()) info_thread.join();
		info_alive = true;
		info_thread = std::thread([this]() {
			while (info_alive) {
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(950ms);
				std::lock_guard<std::mutex> lock(coutmtx);
				if (info_enable) {
					//std::cout << "info string info" << std::endl;
					const auto PV = tree.getPV();
					std::string pvstr;
					if (!PV.empty()) {
						for (int i = 1; i < 7 && i < PV.size() && PV[i] != nullptr; i++) pvstr += PV[i]->move.toUSI()+' ';
						const auto& root = PV[0];
						std::cout << "info pv " << pvstr << "depth " << root->mass << " seldepth " << PV.size()
							<< " score cp " << static_cast<int>(root->eval) << " nodes " << tree.getNodeCount() << std::endl;
					}
					else {
						std::cout << "info string failed to get pv" << std::endl;
					}
				}
			}
		});
	}
}

bool Commander::chakushu() {
	std::lock_guard<std::mutex> lock(coutmtx);
	tree.prohibitSearch();
	info_enable = false;
	const Kyokumen& kyokumen = tree.getRootPlayer().kyokumen;
	if (kyokumen.isDeclarable()) {
		std::cout << "bestmove win" << std::endl;
		return true;
	}
	const SearchNode* const root = tree.rootNode;
	if (root->eval < -33000) {
		std::cout << "bestmove resign" << std::endl;
		return true;
	}
	const auto bestchild = tree.getBestMove();
	if (bestchild == nullptr) {
		tree.permitSearch();
		return false;
	}
	std::cout << "info pv " << bestchild->move.toUSI() << " depth " << bestchild->mass.load() <<
		" score cp " << static_cast<int>(-bestchild->eval) << " nodes " << tree.getNodeCount() << std::endl;
	std::cout << "bestmove " << bestchild->move.toUSI() << std::endl;
	tree.proceed(bestchild);
	if (permitPonder) {
		tree.permitSearch();
	}
	return true;
}