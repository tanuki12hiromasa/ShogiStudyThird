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
		if (tokens.empty()) {
			std::cout << "command ready" << std::endl;
		}
		else if (tokens[0] == "usi") {
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
		}
		else if (tokens[0] == "isready") {
			commander.gameInit();
			std::cout << "readyok" << std::endl;
		}
		else if (tokens[0] == "usinewgame") {
			commander.go_alive = false;
		}
		else if (tokens[0] == "debugsetup") {
			auto setLeaveNodeCommand = usi::split("setoption name leave_branchNode value true", ' ');
			commander.setOption(setLeaveNodeCommand);
			commander.gameInit();
			std::cout << "readyok" << std::endl;
		}
		else if (tokens[0] == "position") {
			commander.go_alive = false;
			commander.position(tokens);
		}
		else if (tokens[0] == "staticevaluate") {
			std::cout << "info cp " << Evaluator::evaluate(commander.tree.getRootPlayer()) << std::endl;
		}
		else if (tokens[0] == "getsfen") {
			std::cout << commander.tree.getRootPlayer().kyokumen.toSfen() << std::endl;
		}
		else if (tokens[0] == "getBanFigure") {
			std::cout << commander.tree.getRootPlayer().kyokumen.toBanFigure() << std::endl;
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
		else if (tokens[0] == "fouttree") {
			commander.tree.foutTree();
			std::cout << "fouttree: done" << std::endl;
		}
		else if (tokens[0] == "ponderhit") {
			//先読みはするがponder機能は利用しない
		}
		else if (tokens[0] == "gameover") {
			commander.go_alive = false;
			commander.info_alive = false;
			commander.stopAgent();
		}
		else if (tokens[0] == "quit") {
			return;
		}
	}
}

Commander::Commander():
	tree(),permitPonder(false)
{
	go_alive = false;
	info_enable = false;
	info_alive = false;
	paramInit();
}

Commander::~Commander() {
	go_alive = false;
	info_enable = false;
	info_alive = false;
	for (auto& ag : agents) {
		ag->terminate();
	}
	if (deleteThread != nullptr && deleteThread->joinable())deleteThread->detach();
	if(go_thread.joinable()) go_thread.join();
	if(info_thread.joinable())info_thread.join();
}

void Commander::coutOption() {
	using namespace std;
	//cout << "option name kppt_filepath type string default ./data/kppt_apery" << endl; //隠しオプション
	cout << "option name leave_branchNode type check default false" << endl;
	cout << "option name NumOfAgent type spin default 12 min 1 max 128" << endl;
	cout << "option name Repetition_score type string default 0" << endl;
	cout << "option name leave_qsearchNode type check default false" << endl;
	cout << "option name QSearch_depth type string default 5" << endl;
	cout << "option name Use_Original_Kyokumen_Eval type check default true" << endl;
	cout << "option name T_choice_const type string default 120" << endl;
	cout << "option name Tc_functionCode type spin default 0 min 0 max 7" << endl;
	cout << "option name T_choice_mass_parent type string default 1" << endl;
	cout << "option name T_choice_children_masses type string default 1" << endl;
	cout << "option name T_eval type string default 40" << endl;
	cout << "option name T_depth type string default 100" << endl;
	cout << "option name Ec_functionCode type spin default 18 min 0 max 19" << endl;
	cout << "option name Ec_c type string default 0.5" << endl;
	cout << "option name NodeMaxNum type string default 100000000" << endl;
	cout << "option name PV_functionCode type spin default 0 min 0 max 2" << endl;
	cout << "option name PV_const type string default 0" << endl;
}

void Commander::setOption(const std::vector<std::string>& token) {
	if (token.size() > 4) {
		if (token[2] == "USI_Ponder") {
			permitPonder = (token[4] == "true");
		}
		else if (token[2] == "leave_branchNode") {
			tree.leave_branchNode = (token[4] == "true");
		}
		else if (token[2] == "kppt_filepath") {
			//aperyのパラメータファイルの位置を指定する 隠しオプション
			apery::apery_feat::folderpath = token[4];
		}
		else if (token[2] == "NumOfAgent") {
			agentNum = std::stoi(token[4]);
		}
		else if (token[2] == "leave_qsearchNode") {
			SearchAgent::setLeaveQSNode(token[4]=="true");
		}
		else if (token[2] == "Repetition_score") {
			SearchNode::setRepScore(std::stod(token[4]));
		}
		else if (token[2] == "QSearch_depth") {
			SearchNode::setMassmaxInQSearch(std::stod(token[4]));
		}
		else if (token[2] == "Use_Original_Kyokumen_Eval") {
			SearchAgent::setUseOriginalKyokumenEval(token[4] == "true");
		}
		else if (token[2] == "T_choice_const") {
			SearchNode::setTcConst(std::stod(token[4]));
		}
		else if (token[2] == "T_choice_mass_parent") {
			SearchNode::setTcmp(std::stod(token[4]));
		}
		else if (token[2] == "T_choice_children_masses") {
			SearchNode::setTcmc(std::stod(token[4]));
		}
		else if (token[2] == "Tc_functionCode") {
			SearchNode::setTcFuncCode(std::stoi(token[4]));
		}
		else if (token[2] == "T_eval") {
			SearchNode::setTeval(std::stod(token[4]));
		}
		else if (token[2] == "T_depth") {
			SearchNode::setTdepth(std::stod(token[4]));
		}
		else if (token[2] == "Ec_functionCode") {
			SearchNode::setEcFuncCode(std::stoi(token[4]));
		}
		else if (token[2] == "Ec_c") {
			SearchNode::setEcC(std::stod(token[4]));
		}
		else if (token[2] == "NodeMaxNum") {
			tree.setNodeMaxsize(std::stoull(token[4]));
		}
		else if (token[2] == "PV_functionCode") {
			SearchNode::setPVFuncCode(std::stoi(token[4]));
		}
		else if (token[2] == "PV_const") {
			SearchNode::setPVConst(std::stod(token[4]));
		}
	}
}

void Commander::paramInit() {
	//usiによる設定前のデフォルト値

	SearchNode::setTdepth(100);
	SearchNode::setTeval(60);
	SearchNode::setMassmaxInQSearch(5);
	tree.setNodeMaxsize(150000000);
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
	info();
}

void Commander::startAgent() {
	assert(agents.empty());
	for (int i = 0; i < agentNum; i++) {
		agents.push_back(std::unique_ptr<SearchAgent>(new SearchAgent(tree, i)));
	}
}
void Commander::stopAgent() {
	for (auto& ag : agents) {
		ag->stop();
	}
}

void Commander::go(const std::vector<std::string>& tokens) {
	//宣言可能かどうかは先に調べる
	const Kyokumen& kyokumen = tree.getRootPlayer().kyokumen;
	if (kyokumen.isDeclarable()) {
		std::cout << "bestmove win" << std::endl;
		return;
	}
	startAgent();
	TimeProperty tp(kyokumen.teban(), tokens);
	go_alive = false;
	if(go_thread.joinable()) go_thread.join();
	go_alive = true;
	go_thread = std::thread([this,tp]() {
		using namespace std::chrono_literals;
		if (tp.rule == TimeProperty::TimeRule::byoyomi && tp.left < 100ms) {
			auto t = tp.added - 150ms;
			std::this_thread::sleep_for(t);
		}
		else {
			std::this_thread::sleep_for(5s);
		}
		chakushu();
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

void Commander::chakushu() {
	std::lock_guard<std::mutex> lock(coutmtx);
	stopAgent();
	info_enable = false;
	const Kyokumen& kyokumen = tree.getRootPlayer().kyokumen;
	if (kyokumen.isDeclarable()) {
		std::cout << "bestmove win" << std::endl;
		return;
	}
	SearchNode* const root = tree.getRoot();
	if (root->eval < -33000) {
		std::cout << "info score cp " << root->eval << std::endl;
		std::cout << "bestmove resign" << std::endl;
		return;
	}
	const auto bestchild = tree.getBestMove();
	if (bestchild == nullptr) {
		std::cout << "info string error no children" << std::endl;
		std::cout << "bestmove resign" << std::endl;
		return;
	}
	std::cout << "info pv " << bestchild->move.toUSI() << " depth " << bestchild->mass.load() <<
		" score cp " << static_cast<int>(-bestchild->eval) << " nodes " << tree.getNodeCount() << std::endl;
	std::cout << "bestmove " << bestchild->move.toUSI() << std::endl;
	tree.proceed(bestchild);
	releaseAgentAndBranch(root, {bestchild});
	if (permitPonder) {
		startAgent();
	}
	return;
}

void Commander::position(const std::vector<std::string>& tokens) {
	stopAgent();
	const auto prevRoot = tree.getRoot();
	auto result = tree.set(tokens);
	if (result.first) {
		releaseAgentAndBranch(prevRoot, std::move(result.second));
	}
	else {
		tree.makeNewTree(tokens);
		releaseAgentAndTree(prevRoot);
	}
}

void Commander::releaseAgentAndBranch(SearchNode* const prevRoot, std::vector<SearchNode*>&& newNodes) {
	auto tmpthread = std::move(deleteThread);
	deleteThread = std::unique_ptr<std::thread>( new std::thread(
		[&tree=tree,prevThread = std::move(tmpthread), prevAgents = std::move(agents), prevRoot, savedNodes = std::move(newNodes)]
		{
			if(prevThread != nullptr && prevThread->joinable()) prevThread->join();
			for (auto& ag : prevAgents) {
				ag->terminate();
			}
			tree.deleteBranch(prevRoot, savedNodes);
		}));
	agents.clear();
}

void Commander::releaseAgentAndTree(SearchNode* const root) {
	auto tmpthread = std::move(deleteThread);
	deleteThread = std::unique_ptr<std::thread>(new std::thread(
		[&tree = tree, prevThread = std::move(tmpthread), prevAgents = std::move(agents), root]
		{
			if (prevThread != nullptr && prevThread->joinable()) prevThread->join();
			for (auto& ag : prevAgents) {
				ag->terminate();
			}
			tree.deleteTree(root);
		}));
	agents.clear();
}