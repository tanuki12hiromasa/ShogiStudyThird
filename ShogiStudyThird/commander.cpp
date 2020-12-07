#include "stdafx.h"
#include "commander.h"
#include "usi.h" 
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

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
			//std::cout << "id name ShibauraSoftmaxThird" << std::endl;
			std::cout << "id name ShibauraSoftmaxThirdJoseki" << std::endl;
#endif
			std::cout << "id author Iwamoto" << std::endl;
			coutOption();
			commander.joseki.printOption();
			commander.yaneuraJoseki.coutOption();
			std::cout << "usiok" << std::endl;
		}
		else if (tokens[0] == "setoption") {
			commander.setOption(tokens);
		}
		else if (tokens[0] == "isready") {
			commander.gameInit();
			commander.joseki.josekiInput(&(commander.tree));
			commander.yaneuraJoseki.readBook();
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
			//エージェントの探索が終わるまで待つ。本来は不要
			if (commander.go_thread.joinable()) commander.go_thread.join();
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
			commander.joseki.setIsSente(commander.tree.getRootPlayer().kyokumen.teban());
			commander.joseki.setNodeCount(commander.tree.nodecount);
			commander.joseki.josekiOutputIGameOver(commander.tree.getHistory(), tokens);
			std::cout << "gameoverok" << std::endl;
		}
		else if (tokens[0] == "quit") {
			return;
		}
		else if (tokens[0] == "yomikomi") {
			commander.joseki.josekiInput(&(commander.tree));
		}
		else if (tokens[0] == "foutjoseki") {
			commander.joseki.setNodeCount(commander.tree.nodecount);
			commander.joseki.josekiOutput(commander.tree.getHistory());
			std::cout << "定跡出力完了" << std::endl;
		}
		else if (tokens[0] == "foutjosekitext") {
			commander.joseki.josekiTextOutput(commander.tree.getHistory());
		}
		else if (tokens[0] == "pruning") {
			std::cout << commander.joseki.pruning(commander.tree.getHistory().front()) << "ノードが削除されました" << std::endl;
		}
		else if (tokens[0] == "yomikomibook") {
			commander.yaneuraJoseki.readBook();
			std::cout << "read book" << std::endl;
		}
		else if (tokens[0] == "makejobanjoseki") {
			commander.makeJobanJoseki(tokens[1], std::stoi(tokens[2]), std::stoi(tokens[3]), std::stoi(tokens[4]));
		}
		else if (tokens[0] == "foutjosekiasyaneura") {
			commander.yaneuraJoseki.outputJosekiAsYaneura(commander.tree.getHistory().front(), tokens[1], std::stoi(tokens[2]));
		}
		else if (tokens[0] == "printsfen") {
			std::cout << commander.tree.getRootPlayer().kyokumen.toSfen() << std::endl;
		}
		else if (tokens[0] == "sfen") {
			std::cout << commander.yaneuraJoseki.getBestMove(usiin).bestMove.toUSI() << std::endl;;
		}
		else if (tokens[0] == "sfenloop") {
			std::ifstream ydb(tokens[1]);
			std::ofstream outdb(tokens[2]);
			std::string dl;
			int kyokumenCount = 0;
			int nullmove = 0;
			int sashiteIttiCount = 0;
			while (!ydb.eof()) {
				std::getline(ydb, dl);
				if (usi::split(dl, ',').size() < 2) {
					break;
				}
				std::string sfen = usi::split(dl, ',')[0] + "";
				if (sfen == "sfen lnsgkgsnl/7b1/p1ppppppp/1r7/7P1/9/PPPPPPP1P/1BG2S1R1/LNS1KG1NL w ") {
					std::cout << "sfen" << std::endl;
				}
				std::string terasyokkubest = usi::split(dl, ',')[1];
				std::string bm = commander.yaneuraJoseki.getBestMove(sfen).bestMove.toUSI();
				bool sashiteitti = terasyokkubest == bm;
				outdb << sfen << "," << terasyokkubest << "," << bm << "," << (sashiteitti ? "TRUE" : "FALSE") << std::endl;
				if (sashiteitti) {
					sashiteIttiCount++;
				}
				if (bm == "nullmove") {
					nullmove++;
				}
				kyokumenCount++;

			}
			int noNullmove = kyokumenCount - nullmove;
			outdb << ",," << nullmove << "," << sashiteIttiCount << std::endl;
			outdb << ",," << kyokumenCount << "," << noNullmove << std::endl;
			outdb << ",," << (double)nullmove / (double)kyokumenCount << "," << (double)sashiteIttiCount / (double)noNullmove << std::endl;
			outdb << ",," << 1.0 - (double)nullmove / (double)kyokumenCount << std::endl;
			outdb << ",," << commander.yaneuraJoseki.getYaneuraJosekiCount() << std::endl;


			ydb.close();
			outdb.close();

			std::ofstream csvsum(tokens[3]);
			std::ofstream summary(tokens[4], std::ios::app);
			csvsum << "局面存在率	最善手一致率	総局面数" << std::endl;
			csvsum << 1.0 - (double)nullmove / (double)kyokumenCount << "	" << (double)sashiteIttiCount / (double)noNullmove << "	" << commander.yaneuraJoseki.getYaneuraJosekiCount() << std::endl;
			summary << 1.0 - (double)nullmove / (double)kyokumenCount << "	" << (double)sashiteIttiCount / (double)noNullmove << "	" << commander.yaneuraJoseki.getYaneuraJosekiCount() << std::endl;
			csvsum.close();
			summary.close();
		}
		else if (tokens[0] == "outputforcsv") {
			std::string foldername = tokens[1];
			std::string filename = tokens[2];
			int start = std::stoi(tokens[3]);
			int count = std::stoi(tokens[4]);
			std::string output;
			for (int i = 0; i < count; ++i) {
				int filenum = i + start;
				int infonum = filenum * 50 + 49;
				output += "ShogiStudyThird.exe\n";
				output += "setoption name joseki_on is true\n";
				output += "setoption name josekifoldername is " + foldername + "\n";
				output += "setoption name josekiinputfilename is joseki" + std::to_string(filenum) + ".bin\n";
				output += "setoption name josekiinputinfofilename is joseki" + std::to_string(infonum) + "_info.txt\n";
				output += "isready\n";
				output += "foutjosekiasyaneura " + foldername + "/yaneurajoseki" + filename + std::to_string(infonum) + ".db 1000000\n";
				output += "yomikomibook " + foldername + "/yaneurajoseki" + filename + std::to_string(infonum) + ".db\n";
				output += "sfenloop yaneuradb.csv " + foldername + "/out" + filename + std::to_string(infonum) + ".csv " + foldername + "/summaryjoseki" + std::to_string(infonum) + ".txt " + foldername + "/summaryAllJoseki.txt\n";
				output += "quit\n";
			}
			std::ofstream of("forcsv.txt");
			std::cout << output << std::endl;
			of << output << std::endl;
			of.close();
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
	cout << "option name continuous_tree type check default true" << endl;
	cout << "option name NumOfAgent type spin default 12 min 1 max 128" << endl;
	cout << "option name Repetition_score type string default 0" << endl;
	cout << "option name leave_qsearchNode type check default false" << endl;
	cout << "option name QSearch_Use_RelativeDepth type check default false" << endl;
	cout << "option name QSearch_depth type string default 0" << endl;
	cout << "option name Use_Original_Kyokumen_Eval type check default false" << endl;
	cout << "option name Ts_disperseFunc type spin default 0 min 0 max 4" << endl;
	cout << "option name Ts_min type string default 40" << endl;
	cout << "option name Ts_max type string default 200" << endl;
	cout << "option name Ts_functionCode type spin default 0 min 0 max 1" << endl;
	cout << "option name Ts_funcParam type string default 1" << endl;
	cout << "option name T_eval type string default 40" << endl;
	cout << "option name T_depth type string default 100" << endl;
	cout << "option name Es_functionCode type spin default 18 min 0 max 20" << endl;
	cout << "option name Es_funcParam type string default 0.5" << endl;
	cout << "option name NodeMaxNum type string default 100000000" << endl;
	cout << "option name PV_functionCode type spin default 0 min 0 max 3" << endl;
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
		else if (token[2] == "continuous_tree") {
			continuousTree = (token[4] == "true");
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
		else if (token[2] == "QSearch_Use_RelativeDepth") {
			SearchAgent::setQSrelativeDepth(token[4] == "true");
		}
		else if (token[2] == "Repetition_score") {
			SearchNode::setRepScore(std::stod(token[4]));
		}
		else if (token[2] == "QSearch_depth") {
			SearchNode::setQSearchDepth(std::stod(token[4]));
		}
		else if (token[2] == "Use_Original_Kyokumen_Eval") {
			SearchAgent::setUseOriginalKyokumenEval(token[4] == "true");
		}
		else if (token[2] == "Ts_min") {
			Ts_min = std::stod(token[4]);
		}
		else if (token[2] == "Ts_max") {
			Ts_max = std::stod(token[4]);
		}
		else if (token[2] == "Ts_disperseFunc") {
			TsDistFuncNum = std::stoi(token[4]);
		}
		else if (token[2] == "Ts_funcParam") {
			SearchNode::setTsFuncParam(std::stod(token[4]));
		}
		else if (token[2] == "Ts_functionCode") {
			SearchNode::setTsFuncCode(std::stoi(token[4]));
		}
		else if (token[2] == "T_eval") {
			SearchNode::setTeval(std::stod(token[4]));
		}
		else if (token[2] == "T_depth") {
			SearchNode::setTdepth(std::stod(token[4]));
		}
		else if (token[2] == "Es_functionCode") {
			SearchNode::setEsFuncCode(std::stoi(token[4]));
		}
		else if (token[2] == "Es_funcParam") {
			SearchNode::setEsFuncParam(std::stod(token[4]));
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
		else {
			joseki.setOption(token);
			yaneuraJoseki.setOption(token);
		}
	}
}

void Commander::paramInit() {
	//usiによる設定前のデフォルト値

	SearchNode::setTdepth(100);
	SearchNode::setTeval(40);
	SearchNode::setQSearchDepth(0);
	tree.setNodeMaxsize(150000000);
	SearchNode::setMateScore(34000);
	SearchNode::setMateOneScore(20);
	SearchNode::setMateScoreBound(30000);
	SearchNode::setRepScore(0);
	agentNum = 12;
}

void Commander::gameInit() {
	if (agents.empty()) {
		BBkiki::init();
		Evaluator::init();
		tree.rootPlayer.feature.set(tree.rootPlayer.kyokumen);
	}
	setTsDistribution();
	info();
}

void Commander::setTsDistribution() {
	TsDistribution.clear();
	switch (TsDistFuncNum) {
		case 0:
			for (int i = 0; i < agentNum; i++) TsDistribution.push_back((Ts_min + Ts_max) / 2);
			break;
		case 1:
		{
			const double delta = (Ts_max - Ts_min) / (agentNum - 1.0);
			for (int i = 0; i < agentNum; i++) TsDistribution.push_back(Ts_min + delta * i);
			break;
		}
		case 2:
		{
			const double minlog = std::log(Ts_min), maxlog = std::log(Ts_max);
			const double delta = (maxlog - minlog) / (agentNum - 1.0);
			for (int i = 0; i < agentNum; i++) TsDistribution.push_back(std::exp(minlog + delta * i));
			break;
		}
		case 3:
		{
			const double c = (Ts_max + Ts_min) / 10.0;
			const double a = 1.0 / (std::exp((Ts_max - Ts_min) / (c * 2.0)) - 1.0);
			for (int i = 0; i < agentNum; i++) {
				const double p = (double)i / (agentNum - 1.0);
				TsDistribution.push_back(c * std::log((p + a) / (1 + a - p)) + (Ts_min + Ts_max) / 2.0);
			}
			break;
		}
		case 4:
		{
			const double minlog = std::log(Ts_min), maxlog = std::log(Ts_max);
			const double c = (minlog + maxlog) / 40.0;
			const double a = 1.0 / (std::exp((maxlog - minlog) / (c * 2.0)) - 1.0);
			for (int i = 0; i < agentNum; i++) {
				const double p = (double)i / (agentNum - 1.0);
				TsDistribution.push_back(std::exp(c * std::log((p + a) / (1 + a - p)) + (minlog + maxlog) / 2.0));
			}
			break;
		}
	}
}

void Commander::startAgent() {
	SearchAgent::resetSimCount();
	assert(agents.empty());
	assert(TsDistribution.size() == agentNum);
	for (int i = 0; i < agentNum; i++) {
		const double Ts = TsDistribution[i];
		agents.push_back(std::unique_ptr<SearchAgent>(new SearchAgent(tree, Ts, i)));
	}
}
void Commander::stopAgent() {
	std::cout << "simcount " << SearchAgent::getSimCount() << std::endl;
	for (auto& ag : agents) {
		ag->stop();
	}
}

void Commander::go(const std::vector<std::string>& tokens) {
	//宣言可能かどうかは先に調べる
	const Kyokumen& kyokumen = tree.getRootPlayer().kyokumen;
	if (kyokumen.isDeclarable()) {
		std::lock_guard<std::mutex> lock(coutmtx);
		std::cout << "bestmove win" << std::endl;
		return;
	}
	else if (tree.getRoot()->eval < -SearchNode::getMateScoreBound()) {
		std::lock_guard<std::mutex> lock(coutmtx);
		std::cout << "bestmove resign" << std::endl;
		return;
	}
	else if (yaneuraJoseki.getJosekiOn()) {
		std::string bestMove = yaneuraJoseki.getBestMoveFromJoseki(kyokumen.toSfen());
		if (bestMove != "nullmove") {
			std::cout << "bestmove " << bestMove << std::endl;
			for (auto node : tree.getRoot()->children) {
				if (node->move.toUSI() == bestMove) {
					tree.proceed(node);
					break;
				}
			}
			return;
		}
	}


	startAgent();
	TimeProperty tp(kyokumen.teban(), tokens);
	go_alive = false;
	if(go_thread.joinable()) go_thread.join();
	go_alive = true;
	go_thread = std::thread([this, tp]() {
		using namespace std::chrono_literals;
		const auto starttime = std::chrono::system_clock::now();
		const SearchNode* root = tree.getRoot();

		//if (joseki.notEndGo(root)) {
		//	std::this_thread::sleep_for(10000ms);
		//};
		if (joseki.deepEnough(root)) {
			chakushu();
		}
		else if (tp.rule == TimeProperty::TimeRule::byoyomi && tp.left < 100ms) {
			do {
				auto t = std::max((tp.added / 5), 50ms);
				std::this_thread::sleep_for(t);
			} while (((std::chrono::system_clock::now() - starttime) < tp.added - 110ms)
				&& std::abs(root->eval) < SearchNode::getMateScoreBound());
			chakushu();
		}
		else {
			std::this_thread::sleep_for(5s);
			chakushu();
		}

		}
	);
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
					if (PV.size() >= 2) {
						for (int i = 1; i < 15 && i < PV.size() && PV[i] != nullptr; i++) pvstr += PV[i]->move.toUSI()+' ';
						const auto& root = PV[0];
						std::cout << std::fixed;
						std::cout << "info pv " << pvstr << "depth " << std::setprecision(2) << root->mass << " seldepth " << (PV.size()-1)
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
	std::lock_guard<std::mutex> clock(coutmtx);
	std::lock_guard<std::mutex> tlock(treemtx);
	stopAgent();
	info_enable = false;
	const Kyokumen& kyokumen = tree.getRootPlayer().kyokumen;
	if (kyokumen.isDeclarable()) {
		std::cout << "bestmove win" << std::endl;
		return;
	}
	SearchNode* const root = tree.getRoot();
	if (root->eval < -33000) {
		std::cout << "info score cp " << static_cast<int>(root->eval) << std::endl;
		std::cout << "bestmove resign" << std::endl;
		return;
	}
	if (joseki.endBattle(root)) {
		std::cout << "info score cp " << static_cast<int>(root->eval) << std::endl;
		std::string rr = static_cast<int>(root->eval) > 0?"win" : "resign";
		std::cout << "bestmove " << rr << std::endl;
		return;
	}
	const auto PV = tree.getPV();
	std::string pvstr;
	if (PV.size() >= 2) {
		for (int i = 1; i < 15 && i < PV.size() && PV[i] != nullptr; i++) pvstr += PV[i]->move.toUSI() + ' ';
		const auto& root = PV[0];
		std::cout << std::fixed;
		std::cout << "info pv " << pvstr << "depth " << std::setprecision(2) << root->mass << " seldepth " << (PV.size() - 1)
			<< " score cp " << static_cast<int>(root->eval) << " nodes " << tree.getNodeCount() << std::endl;
	}
	const auto bestchild = tree.getBestMove();
	if (bestchild == nullptr) {
		//std::cout << "info string error no children" << std::endl;
		std::cout << "bestmove resign" << std::endl;
		return;
	}
	std::cout << "bestmove " << bestchild->move.toUSI() << std::endl;
	tree.proceed(bestchild);
	releaseAgentAndBranch(root, {bestchild});
	if (permitPonder) {
		startAgent();
	}
	return;
}

void Commander::position(const std::vector<std::string>& tokens) {
	std::lock_guard<std::mutex> lock(treemtx);
	stopAgent();
	const auto prevRoot = tree.getRoot();
	if (continuousTree) {
		auto result = tree.set(tokens);
		if (result.first || joseki.getYomikomiOn()) {
			releaseAgentAndBranch(prevRoot, std::move(result.second));
		}
		else {
			//if (!yomikomi_on) {
				tree.makeNewTree(tokens);
				releaseAgentAndTree(prevRoot);
			//}
			//else {
			//	yomikomi_on = false;
			//}
		}
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

#include <Windows.h>
#include <tchar.h>
#include <locale.h>

void Commander::yomikomi()
{


	std::string ss;
	int i = 0, j = 0;
	size_t i_max = 0;
	std::vector<SearchNode*> test;

	SearchNode* node = NULL;
	std::vector<int> parents = {};
	parents.push_back(-1);
	int index = -1;
	int	st = 0;
	uint16_t usiU;
	double eval = 0.0;
	double mass = 0.0;
	Move move;
	std::vector<int> childIndex;
	std::cout << "start \"Yomikomi!\" " << std::endl;
	static int num = 0;


	if (true) {
		std::ifstream ifs;
		ifs.open("treelog.txt");
		if (ifs.fail()) {
			//std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
		}
		std::getline(ifs, ss); //sfen
		//std::getline(ifs, ss); //sfen
		//std::string sfen = "position " + ss;
		//tree.makeNewTree(usi::split(sfen, ' '));

		while (1) {

			std::getline(ifs, ss);
			if (ifs.eof()) {
				break;//ファイルの終わりならブレイク
			}


			auto split = usi::split(ss, ',');
			index = std::stoi(split[0]);
			st = std::stoi(split[1]);
			if (split[2] != " nullmove") {
				//usiU = std::stoi(split[2]);
				move = Move(split[2],true);
			}
			else {
				usiU = koma::Position::NullMove;
				move = Move(usiU);
			}
			eval = std::stod(split[3]);
			mass = std::stod(split[4]);
			int ind = 6;
			childIndex.clear();
			while (split[ind] != "]") {
				childIndex.push_back(std::stoi(split[ind]));
				parents.push_back(index);
				++ind;
			}

			//move = Move(usiU);



			if (index == 0) {//1つ目は親なし
				test.push_back(node->restoreNode(move, (SearchNode::State)st, eval, mass));
			}
			else {
				test.push_back(node->restoreNode(move, (SearchNode::State)st, eval, mass));
				test[parents[index]]->children.push_back(test[index]);
			}

			i++;
		}
	}

	std::cout << "end \"Yomikomi!\" " << std::endl;
	i_max = i;
	std::vector<std::string> startpos;
	startpos.push_back("position");
	startpos.push_back("startpos");
	Kyokumen kyo = Kyokumen(startpos);

	node = test[0];
	tree.setRoot(node, kyo, i_max);

}

//定跡を入れるファイル名と、定跡つくりのための実行回数、序盤定跡の深さを指定
void Commander::makeJobanJoseki(std::string folderName,int count,int depth,int second){
	for (int i = 0; i < count; ++i) {
		std::cout << i + 1 << "回目" << std::endl;
		joseki.setInputFileName(folderName + "\\joseki" + std::to_string(i));
		joseki.setOutputFileName(folderName + "\\joseki" + std::to_string(i + 1));
		gameInit();
		joseki.josekiInput(&(tree));
		
		std::vector<std::string>tokens = {"go","btime","0","wtime","0","byoyomi",std::to_string(second)};
		for (int j = 0; j < depth; ++j) {
			go(tokens);
			//エージェントの探索が終わるまで待つ。本来は不要
			if (go_thread.joinable()) go_thread.join();
		}
		
		joseki.josekiOutput(tree.getHistory());
		tree.leave_branchNode = false;
		releaseAgentAndTree(tree.getRoot());
		tree.leave_branchNode = true;
	}
}
