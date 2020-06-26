#include "stdafx.h"
#include "commander.h"
#include "usi.h" 
#include "joseki.h"
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
			//定跡を利用する場合のみ読みこみ
			if (commander.yomikomi_on) {
				//Joseki joseki;
				//joseki.josekiInput();
				//commander.tree.setRoot(joseki.getJosekiNodes(), joseki.getKyokumen(), joseki.getChildCount());
				commander.joseki.readBook("joseki/user_book1.db");
				std::cout << "read book" << std::endl;

			}
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
			if (commander.joseki_make_type == 1) {
				commander.tree.foutJoseki(commander.joseki_make_type, commander.joseki_file_count);
			}
			std::cout << "gameoverok" << std::endl;
		}
		else if (tokens[0] == "quit") {
			return;
		}
		else if (tokens[0] == "yomikomi") {
			Joseki joseki;
			joseki.josekiInput();
			commander.tree.setRoot(joseki.getJosekiNodes(), joseki.getKyokumen(), joseki.getChildCount());
			std::cout << "yomikomi: done" << std::endl;
		}
		else if (tokens[0] == "foutjoseki") {
			Joseki joseki;
			joseki.josekiOutput(commander.tree.getHistory().front(),commander.tree.startKyokumen);

			std::cout << "foutjoseki: done" << std::endl;
		}
		else if (tokens[0] == "yomikomibook") {
			commander.joseki.readBook("joseki/user_book1.db");
			std::cout << "read book" << std::endl;
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
	cout << "option name yomikomi_on type check default false" << endl;
	cout << "option name yomikomi_file_name type string default treemake" << endl;
	cout << "option name yomikomi_type type string default 0" << endl;
	cout << "option name joseki_make_type type string default 0" << endl;

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
		else if (token[2] == "yomikomi_on") {
			yomikomi_on = (token[4] == "true");
		}
		else if (token[2] == "yomikomi_file_name") {
			yomikomi_file_name = token[4];
		}
		else if (token[2] == "yomikomi_type") {
			yomikomi_type = std::stoi(token[4]);
		}
		else if (token[2] == "joseki_make_type") {
			joseki_make_type = std::stoi(token[4]);
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

	if (yomikomi_on) {
		//Softmax以外の定跡を読み込んであったら、それを利用する
		auto bm = joseki.getBestMove(Joseki::getSfenTrimed(kyokumen.toSfen()));
		if (bm.num != -1) {
			auto bestChild = bm.bestMove;
			std::cout << "bestmove " << bestChild.toUSI() << std::endl;
			return;
		}
	}

	startAgent();
	TimeProperty tp(kyokumen.teban(), tokens);
	go_alive = false;
	if(go_thread.joinable()) go_thread.join();
	go_alive = true;
	go_thread = std::thread([this,tp]() {
		using namespace std::chrono_literals;
		const auto starttime = std::chrono::system_clock::now();
		const SearchNode* root = tree.getRoot();
		if (tp.rule == TimeProperty::TimeRule::byoyomi && tp.left < 100ms) {
			do {
				auto t = std::max((tp.added / 5), 50ms);
				std::this_thread::sleep_for(t);
			} while (((std::chrono::system_clock::now()-starttime) < tp.added - 110ms)
				&& std::abs(root->eval) < SearchNode::getMateScoreBound());
			chakushu();
		}
		else {
			std::this_thread::sleep_for(5s);
			chakushu();
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
		if (result.first) {
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
//
//#include <Windows.h>
//#include <tchar.h>
//#include <locale.h>
//
//time_t yomikomistart;
//time_t yomikomijikan = 0;
//time_t hukugenstart;
//time_t hukugenjikan = 0;
//time_t getlinestart;
//time_t getlinejikan = 0;
//
////1行読みこむ。返り値は子供のindexであってbeginでないことに注意
//static std::vector<int> yomikomiLine(const char* const lines, SearchNode**& sn, int*& parents, const int index, const size_t length, const int offset) {
//	yomikomistart = clock();
//
//	std::vector<int> childIndexes;
//	//\0の分+1
//	char* cstr = (char*)malloc(sizeof(char) * (length + 1));
//	//\rの分+1
//	size_t ind = offset + index * (length);
//	//読むのは\rの前まで
//	memcpy_s(cstr, length + 1, &(lines[ind]), length);
//	cstr[length] = '\0';
//	//std::string ss(cstr);
//
//	getlinejikan += clock() - yomikomistart;
//
//	int st;
//	uint16_t usiU;
//	double eval;
//	double mass;
//	int childCount;
//	int childIndex;
//
//	sscanf_s(cstr, "%d,%d,%d,%lf,%lf,%d,%d",&index, &st, &usiU, &eval, &mass, &childCount,&childIndex);
//
//	Move move = Move(usiU);
//
//	for (int i = 0; i < childCount; ++i) {		//子ノードのインデックスが読み終わるまでループ
//		childIndexes.push_back(childIndex + i);
//		parents[childIndexes.back()] = index;	 //親のインデックスを要素として持つ
//	}
//
//	yomikomijikan += (clock() - yomikomistart);
//	hukugenstart = clock();
//
//	sn[index] = (SearchNode::restoreNode(move, st, eval, mass));
//	if (index != 0) {//1つ目は親なし
//		sn[parents[index]]->children.push_back(sn[index]);
//	}
//	sn[index]->children.reserve(childCount);
//
//	hukugenjikan += (clock() - hukugenstart);
//
//	free(cstr);
//
//	return childIndexes;
//}
//
////indexとその子供を再帰的に読みこむ
//static void yomikomiRecursive(const char* const lines, SearchNode**& sn, int*& parents, const int index, const int length, const int offset) {
//	auto children = yomikomiLine(lines, sn, parents, index, length, offset);
//	for (auto c : children) {
//		yomikomiRecursive(lines, sn, parents, c, length, offset);
//	}
//}
//
////指定された深さまで読みこみ、子供たちを返す。0で指定されたindexのみ。並列処理はしない
//static std::vector<int> yomikomiDepth(const char* const lines, SearchNode**& sn, int*& parents, const int index, const int length, const int offset,const int depth) {
//	auto c = yomikomiLine(lines, sn, parents, index, length, offset);
//	if (depth == 0) {
//		return c;
//	}
//	else {
//		std::vector<int>children;
//		for (auto ch : c) {
//			auto v = yomikomiDepth(lines, sn, parents, ch, length, offset, depth - 1);
//			children.insert(children.end(),v.begin(),v.end());
//		}
//		return children;
//	}
//}
//
//
////1行読みこむ。fpをそのまま渡す用
//static std::vector<int> yomikomiLineF(FILE* fp, SearchNode**& sn, int*& parents, const int index, const size_t length, const int offset) {
//	yomikomistart = clock();
//
//	std::vector<int> childIndexes;
//	//\0の分+1
//	char* cstr = (char*)malloc(sizeof(char) * (length + 1));
//	//\rの分+1
//	fpos_t ind = offset + index * (length);
//	//読むのは\rの前まで
//	fsetpos(fp, &ind);
//	//memcpy_s(cstr, length + 1, &(lines[ind]), length);
//	//cstr[length] = '\0';
//	//std::string ss(cstr);
//
//	getlinejikan += clock() - yomikomistart;
//
//	int tIndex;
//	int st;
//	uint16_t usiU;
//	double eval;
//	double mass;
//	int childCount;
//	int childIndex;
//
//	//sscanf_s(cstr, "%d,%d,%d,%lf,%lf,%d,%d", &index, &st, &usiU, &eval, &mass, &childCount, &childIndex);
//	fread(&tIndex, sizeof(index), 1, fp);
//	fread(&st, sizeof(st), 1, fp);
//	fread(&usiU, sizeof(usiU), 1, fp);
//	fread(&eval, sizeof(eval), 1, fp);
//	fread(&mass, sizeof(mass), 1, fp);
//	fread(&childCount, sizeof(childCount), 1, fp);
//	fread(&childIndex, sizeof(childIndex), 1, fp);
//
//	Move move = Move(usiU);
//
//	for (int i = 0; i < childCount; ++i) {		//子ノードのインデックスが読み終わるまでループ
//		childIndexes.push_back(childIndex + i);
//		parents[childIndexes.back()] = index;	 //親のインデックスを要素として持つ
//	}
//
//	yomikomijikan += (clock() - yomikomistart);
//	hukugenstart = clock();
//
//	sn[index] = (SearchNode::restoreNode(move, st, eval, mass));
//	if (index != 0) {//1つ目は親なし
//		sn[parents[index]]->children.push_back(sn[index]);
//	}
//	sn[index]->children.reserve(childCount);
//
//	hukugenjikan += (clock() - hukugenstart);
//
//	//free(cstr);
//
//	return childIndexes;
//}
//
////indexとその子供を再帰的に読みこむ
//static void yomikomiRecursiveF(FILE *fp, SearchNode**& sn, int*& parents, const int index, const int length, const int offset) {
//	auto children = yomikomiLineF(fp, sn, parents, index, length, offset);
//	for (auto c : children) {
//		yomikomiRecursiveF(fp, sn, parents, c, length, offset);
//	}
//}
//
////指定された深さまで読みこみ、子供たちを返す。0で指定されたindexのみ。並列処理はしない
//static std::vector<int> yomikomiDepthF(FILE *fp, SearchNode**& sn, int*& parents, const int index, const int length, const int offset, const int depth) {
//	auto c = yomikomiLineF(fp, sn, parents, index, length, offset);
//	if (depth == 0) {
//		return c;
//	}
//	else {
//		std::vector<int>children;
//		for (auto ch : c) {
//			auto v = yomikomiDepthF(fp, sn, parents, ch, length, offset, depth - 1);
//			children.insert(children.end(), v.begin(), v.end());
//		}
//		return children;
//	}
//}
//
//
//
//void Commander::yomikomi()
//{
//	yomikomijikan = 0;
//	hukugenjikan = 0;
//	getlinejikan = 0;
//	//実行時間計測用
//	time_t startTime = clock();
//	time_t fileYomikomi;
//	time_t tansakugiYomikomi;
//	time_t endTime;
//
//	int i = 0, j = 0;
//	//読みこむ木の入ったファイルを開く
//	std::string fileName = (yomikomi_file_name + ".txt");
//	if(joseki_make_type == 1){
//		WIN32_FIND_DATA findFileData;
//		HANDLE hFind;
//		auto target = L"josekiFolder/*.txt";
//		int fileCount = -1;
//		hFind = FindFirstFile(target, &findFileData);
//		if (hFind != INVALID_HANDLE_VALUE) {
//			do {
//				++fileCount;
//				//_tprintf(TEXT("%d: %s\n"), ++fileCount, findFileData.cFileName);
//			} while (FindNextFile(hFind, &findFileData));
//			FindClose(hFind);
//		}
//		fileName = "josekiFolder/treejoseki" + std::to_string(fileCount) + ".txt";
//		joseki_file_count = fileCount;
//	}
//
//	//メモリマップドファイルで開いてみる
//	//ファイルオープン
//	WCHAR *wStrW;
//	wStrW = (WCHAR*)malloc((fileName.length() + 1) * sizeof(WCHAR));
//	size_t wLen = 0;
//	errno_t err = 0;
//
//	//ロケール指定
//	//setlocale(LC_ALL, "japanese");
//	//変換
//	err = mbstowcs_s(&wLen, wStrW, fileName.length() + 1, fileName.c_str(), _TRUNCATE);
//	HANDLE hFile;
//	hFile = CreateFile(wStrW, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
//	if (hFile == INVALID_HANDLE_VALUE) {
//		std::cout << GetLastError() << std::endl;
//	}
//	
//	//ファイルマッピングオブジェクトを作成
//	HANDLE hMap;
//	auto mapname = _T("TestFile");
//	hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, mapname);
//	if (hMap <= 0) {
//		std::cout << "get pointa error" << std::endl;
//	}
//
//	//ファイルポインタを取得
//	char* pPointer;
//	int pointerIndex = 0;
//	pPointer = (char*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
//
//	fileYomikomi = clock() - startTime;
//
//	//行の数。1行目はsfenなため1引いてある
//	char* firstLine = (char*)malloc(sizeof(char) * 128);
//	int flSize = 0;
//	while (pPointer[pointerIndex] != '\n') {
//		firstLine[flSize] = pPointer[pointerIndex];
//		flSize++;
//		pointerIndex++;
//	}
//	firstLine[flSize] = '\0';
//	pointerIndex++;
//	auto flv = usi::split(firstLine, ',');
//	int lineCount = std::stoi(flv[3]);
//	int ColumnSize = std::stoi(flv[1]);
//	//ノードと親の領域を必要な数だけ確保
//	SearchNode** nodes = (SearchNode**)malloc(sizeof(SearchNode*) * lineCount);
//	int* parents = (int*)malloc(sizeof(int) * lineCount);
//
//	//ノードの部分まで進行
//	while (pPointer[pointerIndex] != '\n') {
//		pointerIndex++;
//	}
//	int offset = ++pointerIndex;
//
//	std::cout << "start \"Yomikomi!\" " << std::endl;
//
//	int depth = 0;
//	std::vector<int>childrenToThread;
//	
//	if (yomikomi_type == 0) {
//		childrenToThread = yomikomiDepth(pPointer, nodes, parents, 0, ColumnSize, offset, depth);
//	}
//	else if (yomikomi_type) {
//		childrenToThread.push_back(0);
//	}
//
//	//再帰的に読みこむ
//	std::vector<std::thread> thr;
//	for (auto c : childrenToThread) {
//		thr.push_back(std::thread(yomikomiRecursive, pPointer, std::ref(nodes), std::ref(parents), c, ColumnSize, offset));
//	}
//	for (int i = 0; i < thr.size();++i) {
//		thr[i].join();
//	}
//	tansakugiYomikomi = clock() - fileYomikomi - startTime;
//
//	//depth直後が評価値順ではないので修正する
//	SearchNode::sortChildren(nodes[0]);
//	
//	std::cout << "end \"Yomikomi!\" " << std::endl;
//	
//	//初期局面の作成(まだ初期状態から弄ってない)
//	std::vector<std::string> startpos;
//	startpos.push_back(" ");
//	startpos.push_back("startpos");
//	Kyokumen kyo = Kyokumen(startpos);
//
//	tree.setRoot(nodes[0], kyo, lineCount);
//
//	//ファイルを閉じる
//	if (UnmapViewOfFile(pPointer) == 0) {
//		std::cout << "ファイル綴じエラー" << std::endl;
//	}
//
//	CloseHandle(hMap);
//	CloseHandle(hFile);
//
//	free(wStrW);
//	free(firstLine);
//	free(nodes);
//	free(parents);
//
//	endTime = clock() - startTime;
//
//	std::ofstream ofs("time.txt",std::ios::app);
//	ofs << "読み込みタイプ," << yomikomi_type << ",ノード数," << lineCount << ",読み込み時間," << yomikomijikan / (double)CLOCKS_PER_SEC << ",復元時間," << hukugenjikan / (double)CLOCKS_PER_SEC << ",ファイル読み込み完了時刻," << fileYomikomi / (double)CLOCKS_PER_SEC << ",探索木読み込み完了時刻," << tansakugiYomikomi / (double)CLOCKS_PER_SEC << ",行読み時間," << getlinejikan / (double)CLOCKS_PER_SEC << ",総時間," << endTime / (double)CLOCKS_PER_SEC << "," << fileName << std::endl;
//	ofs.close();
//}
//
//void Commander::yomikomiKizon()
//{
//	yomikomijikan = 0;
//	hukugenjikan = 0;
//	getlinejikan = 0;
//
//	//実行時間計測用
//	time_t startTime = clock();
//	time_t fileYomikomi = 0;
//	time_t tansakugiYomikomi = 0;
//	time_t endTime = 0;
//
//	std::string ss;
//	int i = 0, j = 0;
//	size_t i_max = 0;
//	std::vector<SearchNode*> test;
//
//	SearchNode* node = NULL;
//	std::vector<int> parents = {};
//	parents.push_back(-1);
//	int index = 0;
//	int	st = 0;
//	uint16_t usiU;
//	double eval = 0.0;
//	double mass = 0.0;
//	Move move;
//	int childCount;
//	std::cout << "start \"Yomikomi!\" " << std::endl;
//	static int num = 0;
//
//	if (yomikomi_type == 1) {
//		std::ifstream ifs;
//		ifs.open(yomikomi_file_name + ".txt");
//		if (ifs.fail()) {
//			std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
//		}
//		std::getline(ifs, ss); //sfen
//		std::getline(ifs, ss); //sfen
//		//std::string sfen = "position " + ss;
//		//tree.makeNewTree(usi::split(sfen, ' '));
//		fileYomikomi = clock() - startTime;
//
//		while (1) {
//			yomikomistart = clock();
//
//			std::getline(ifs, ss);
//			if (ifs.eof()) {
//				break;//ファイルの終わりならブレイク
//			}
//
//			getlinejikan += clock() - yomikomistart;
//
//			auto split = usi::split(ss, ',');
//			index = std::stoi(split[0]);
//			st = std::stoi(split[1]);
//			usiU = std::stoi(split[2]);
//			eval = std::stod(split[3]);
//			mass = std::stod(split[4]);
//			childCount = std::stoi(split[5]);
//
//			move = Move(usiU);
//
//			for (int j = 0; j < childCount; ++j) {		//子ノードのインデックスが読み終わるまでループ
//				parents.push_back(index);	 //親のインデックスを要素として持つ
//			}
//
//			yomikomijikan += clock() - yomikomistart;
//			hukugenstart = clock();
//
//			if (index == 0) {//1つ目は親なし
//				test.push_back(node->restoreNode(move, st, eval, mass));
//			}
//			else {
//				test.push_back(node->restoreNode(move, st, eval, mass));
//				test[parents[index]]->children.push_back(test[index]);
//			}
//
//			hukugenjikan += clock() - hukugenstart;
//			i++;
//		}
//	}
//	else if (yomikomi_type == 2) {
//		std::ifstream ifs;
//		ifs.open(yomikomi_file_name + ".txt");
//		if (ifs.fail()) {
//			std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
//		}
//		std::getline(ifs, ss); //sfen
//		std::getline(ifs, ss); //sfen
//		//std::string sfen = "position " + ss;
//		//tree.makeNewTree(usi::split(sfen, ' '));
//		fileYomikomi = clock() - startTime;
//
//		while (1) {
//			yomikomistart = clock();
//
//			std::getline(ifs, ss);
//			if (ifs.eof()) {
//				break;//ファイルの終わりならブレイク
//			}
//
//			getlinejikan += clock() - yomikomistart;
//
//			sscanf_s(ss.c_str(), "%d,%d,%d,%lf,%lf,%d", &index, &st, &usiU, &eval, &mass, &childCount);
//			move = Move(usiU);
//
//			for (int j = 0; j < childCount; ++j) {		//子ノードのインデックスが読み終わるまでループ
//				parents.push_back(index);	 //親のインデックスを要素として持つ
//			}
//
//			yomikomijikan += clock() - yomikomistart;
//			hukugenstart = clock();
//
//			if (index == 0) {//1つ目は親なし
//				test.push_back(node->restoreNode(move, st, eval, mass));
//			}
//			else {
//				test.push_back(node->restoreNode(move, st, eval, mass));
//				test[parents[index]]->children.push_back(test[index]);
//			}
//
//			hukugenjikan += clock() - hukugenstart;
//			i++;
//		}
//	}
//	else if (yomikomi_type == 3) {
//		const int N = 256;
//		char buf[N];
//		FILE* fp;
//		errno_t err = fopen_s(&fp, (yomikomi_file_name + ".txt").c_str(), "r");
//		if (err) {
//			std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
//		}
//
//		fgets(buf, N, fp);
//		fgets(buf, N, fp);
//		//std::string sfen = "position " + ss;
//		//tree.makeNewTree(usi::split(sfen, ' '));
//		fileYomikomi = clock() - startTime;
//
//		while (1) {
//			yomikomistart = clock();
//
//			if (fgets(buf, N, fp) == NULL) {
//				break;//ファイルの終わりならブレイク
//			}
//
//			getlinejikan += clock() - yomikomistart;
//
//			sscanf_s(buf, "%d,%d,%d,%lf,%lf,%d", &index, &st, &usiU, &eval, &mass, &childCount);
//			move = Move(usiU);
//
//			for (int j = 0; j < childCount; ++j) {		//子ノードのインデックスが読み終わるまでループ
//				parents.push_back(index);	 //親のインデックスを要素として持つ
//			}
//
//			yomikomijikan += clock() - yomikomistart;
//			hukugenstart = clock();
//
//			if (index == 0) {//1つ目は親なし
//				test.push_back(node->restoreNode(move, st, eval, mass));
//			}
//			else {
//				test.push_back(node->restoreNode(move, st, eval, mass));
//				test[parents[index]]->children.push_back(test[index]);
//			}
//
//			hukugenjikan += clock() - hukugenstart;
//			i++;
//		}
//		fclose(fp);
//	}
//	else if (yomikomi_type == 4) {
//		const int N = sizeof(char) * 1024 * 1024;
//		std::string ss;
//		char* buf = (char*)malloc(N);
//		FILE* fp;
//		errno_t err = fopen_s(&fp, (yomikomi_file_name + ".txt").c_str(), "r");
//		if (err) {
//			std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
//		}
//
//		fgets(buf, N, fp);
//		fgets(buf, N, fp);
//
//		while (!feof(fp)) {
//			size_t size = fread(&buf[0], sizeof(char), N - 1, fp);
//			buf[size] = '\0';
//			ss += buf;
//		}
//		fclose(fp);
//		free(buf);
//
//		auto splitss = usi::split(ss, '\n');
//
//		//std::string sfen = "position " + ss;
//		//tree.makeNewTree(usi::split(sfen, ' '));
//		fileYomikomi = clock() - startTime;
//
//		while (i <= splitss.size()) {
//			yomikomistart = clock();
//
//			getlinejikan += clock() - yomikomistart;
//
//			sscanf_s(splitss[i].c_str(), "%d,%d,%d,%lf,%lf,%d", &index, &st, &usiU, &eval, &mass, &childCount);
//			move = Move(usiU);
//
//
//			for (int j = 0; j < childCount; ++j) {		//子ノードのインデックスが読み終わるまでループ
//				parents.push_back(index);	 //親のインデックスを要素として持つ
//			}
//
//			yomikomijikan += clock() - yomikomistart;
//			hukugenstart = clock();
//
//			if (index == 0) {//1つ目は親なし
//				test.push_back(node->restoreNode(move, st, eval, mass));
//			}
//			else {
//				test.push_back(node->restoreNode(move, st, eval, mass));
//				test[parents[index]]->children.push_back(test[index]);
//			}
//
//			hukugenjikan += clock() - hukugenstart;
//			i++;
//		}
//	}
//	else if (yomikomi_type == 5) {
//		std::string ss;
//		FILE* fp;
//		errno_t err = fopen_s(&fp, (yomikomi_file_name + ".txt").c_str(), "r");
//		if (err) {
//			std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
//		}
//		char firstLine[256];
//		fgets(firstLine, 256, fp);
//		size_t lineLength = std::stoi(usi::split(firstLine, ',')[1]);
//		size_t lineSize = std::stoi(usi::split(firstLine, ',')[3]);
//
//		const size_t N = sizeof(char) * (lineLength);
//		char* buf = (char*)malloc(N);
//
//		fgets(buf, 256, fp);
//
//		std::vector<std::string> splitss;
//		splitss.reserve(lineSize + 2);
//
//		while (!feof(fp)) {
//			size_t size = fread(&buf[0], sizeof(char), N - 1, fp);
//			buf[size] = '\0';
//			splitss.push_back(std::string(buf));
//		}
//		fclose(fp);
//		free(buf);
//
//		//auto splitss = usi::split(ss, '\n');
//
//		//std::string sfen = "position " + ss;
//		//tree.makeNewTree(usi::split(sfen, ' '));
//		fileYomikomi = clock() - startTime;
//
//		while (i < splitss.size()) {
//			if (splitss[i].length() == 0) {
//				break;
//			}
//
//			yomikomistart = clock();
//
//			getlinejikan += clock() - yomikomistart;
//
//			sscanf_s(splitss[i].c_str(), "%d,%d,%d,%lf,%lf,%d", &index, &st, &usiU, &eval, &mass, &childCount);
//			move = Move(usiU);
//
//
//			for (int j = 0; j < childCount; ++j) {		//子ノードのインデックスが読み終わるまでループ
//				parents.push_back(index);	 //親のインデックスを要素として持つ
//			}
//
//			yomikomijikan += clock() - yomikomistart;
//			hukugenstart = clock();
//
//			if (index == 0) {//1つ目は親なし
//				test.push_back(node->restoreNode(move, st, eval, mass));
//			}
//			else {
//				test.push_back(node->restoreNode(move, st, eval, mass));
//				test[parents[index]]->children.push_back(test[index]);
//			}
//
//			hukugenjikan += clock() - hukugenstart;
//			i++;
//		}
//	}
//	else if (yomikomi_type == 6) {
//		std::string ss;
//		FILE* fp;
//		errno_t err = fopen_s(&fp, (yomikomi_file_name + ".txt").c_str(), "r");
//		if (err) {
//			std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
//		}
//		int offset = 0;
//		char firstLine[256];
//		fgets(firstLine, 256, fp);	//1行目
//		//offset += strlen(firstLine);
//		size_t lineLength = std::stoi(usi::split(firstLine, ',')[1]) - 1;
//		size_t lineSize = std::stoi(usi::split(firstLine, ',')[3]);
//
//		const size_t fullSize = sizeof(char) * (lineLength + 2) * lineSize + 1;
//		const size_t N = fullSize;
//		char* buf = (char*)malloc(N);
//
//		fgets(buf, 256, fp);	//2行目、sfen
//		//offset += strlen(buf);
//
//		char* splitss = (char*)malloc(fullSize);
//		memset(splitss, 0, fullSize);
//
//		while (!feof(fp)) {
//			size_t size = fread(&buf[0], sizeof(char), N - 1, fp);
//			buf[size] = '\0';
//			strcat_s(splitss, fullSize, buf);
//		}
//		fclose(fp);
//		free(buf);
//
//		SearchNode** tNode = (SearchNode**)malloc(sizeof(SearchNode*) * lineSize);
//		int* tParents = (int*)malloc(sizeof(int) * lineSize);
//		//auto splitss = usi::split(ss, '\n');
//
//		//std::string sfen = "position " + ss;
//		//tree.makeNewTree(usi::split(sfen, ' '));
//		fileYomikomi = clock() - startTime;
//
//		int depth = 0;
//		std::vector<int>childrenToThread;
//		//childrenToThread = yomikomiDepth(splitss, tNode, tParents, 0, lineLength, offset, depth);
//		childrenToThread.push_back(0);
//
//		//再帰的に読みこむ
//		std::vector<std::thread> thr;
//		for (auto c : childrenToThread) {
//			thr.push_back(std::thread(yomikomiRecursive, splitss, std::ref(tNode), std::ref(tParents), c, lineLength, offset));
//		}
//		for (int i = 0; i < thr.size(); ++i) {
//			thr[i].join();
//		}
//		//yomikomiRecursive(splitss, tNode, tParents, 0, lineLength, offset);
//
//		test.push_back(tNode[0]);
//		i = lineSize;
//
//		free(tNode);
//		free(tParents);
//	}
//	else if (yomikomi_type == 7) {
//		std::string ss;
//		FILE* fp;
//		errno_t err = fopen_s(&fp, (yomikomi_file_name + ".txt").c_str(), "r");
//		if (err) {
//			std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
//		}
//		int offset = 0;
//		char firstLine[256];
//		fgets(firstLine, 256, fp);	//1行目
//		//offset += strlen(firstLine);
//		size_t lineLength = std::stoi(usi::split(firstLine, ',')[1]) - 1;
//		size_t lineSize = std::stoi(usi::split(firstLine, ',')[3]);
//
//		const size_t fullSize = sizeof(char) * (lineLength + 2) * lineSize + 1;
//		const size_t N = fullSize;
//		char* buf = (char*)malloc(N);
//
//		fgets(buf, 256, fp);	//2行目、sfen
//		//offset += strlen(buf);
//
//		char* splitss = (char*)malloc(fullSize);
//		memset(splitss, 0, fullSize);
//
//		while (!feof(fp)) {
//			size_t size = fread(&buf[0], sizeof(char), N - 1, fp);
//			buf[size] = '\0';
//			strcat_s(splitss, fullSize, buf);
//		}
//		fclose(fp);
//		free(buf);
//
//		SearchNode** tNode = (SearchNode**)malloc(sizeof(SearchNode*) * lineSize);
//		int* tParents = (int*)malloc(sizeof(int) * lineSize);
//		//auto splitss = usi::split(ss, '\n');
//
//		//std::string sfen = "position " + ss;
//		//tree.makeNewTree(usi::split(sfen, ' '));
//		fileYomikomi = clock() - startTime;
//
//		int depth = 0;
//		std::vector<int>childrenToThread;
//		childrenToThread = yomikomiDepth(splitss, tNode, tParents, 0, lineLength, offset, depth);
//		//childrenToThread.push_back(0);
//
//		//再帰的に読みこむ
//		std::vector<std::thread> thr;
//		for (auto c : childrenToThread) {
//			thr.push_back(std::thread(yomikomiRecursive, splitss, std::ref(tNode), std::ref(tParents), c, lineLength, offset));
//		}
//		for (int i = 0; i < thr.size(); ++i) {
//			thr[i].join();
//		}
//		//yomikomiRecursive(splitss, tNode, tParents, 0, lineLength, offset);
//
//		test.push_back(tNode[0]);
//		i = lineSize;
//
//		free(tNode);
//		free(tParents);
//	}
//	else if (yomikomi_type == 11) {
//		std::string ss;
//		FILE* fp;
//		errno_t err = fopen_s(&fp, (yomikomi_file_name + ".bin").c_str(), "rb");
//		if (err) {
//			std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
//		}
//		int offset = 0;
//		char firstLine[256];
//		fgets(firstLine, 256, fp);	//1行目
//		offset += strlen(firstLine);
//		size_t lineLength = std::stoi(usi::split(firstLine, ',')[1]) - 1;
//		size_t lineSize = std::stoi(usi::split(firstLine, ',')[3]);
//
//		const size_t fullSize = sizeof(char) * (lineLength + 2) * lineSize + 1;
//		const size_t N = fullSize;
//		char* buf = (char*)malloc(N);
//
//		fgets(buf, 256, fp);	//2行目、sfen
//		offset += strlen(buf);
//
//		
//		SearchNode** tNode = (SearchNode**)malloc(sizeof(SearchNode*) * lineSize);
//		int* tParents = (int*)malloc(sizeof(int) * lineSize);
//		//auto splitss = usi::split(ss, '\n');
//
//		//std::string sfen = "position " + ss;
//		//tree.makeNewTree(usi::split(sfen, ' '));
//		fileYomikomi = clock() - startTime;
//
//		int depth = 0;
//		std::vector<int>childrenToThread;
//		childrenToThread = yomikomiDepthF(fp, tNode, tParents, 0, lineLength, offset, depth);
//		//childrenToThread.push_back(0);
//
//		//再帰的に読みこむ
//		std::vector<std::thread> thr;
//		for (auto c : childrenToThread) {
//			thr.push_back(std::thread(yomikomiRecursiveF, fp, std::ref(tNode), std::ref(tParents), c, lineLength, offset));
//		}
//		for (int i = 0; i < thr.size(); ++i) {
//			thr[i].join();
//		}
//		//yomikomiRecursive(splitss, tNode, tParents, 0, lineLength, offset);
//
//		test.push_back(tNode[0]);
//		i = lineSize;
//
//		free(tNode);
//		free(tParents);
//	}
//	tansakugiYomikomi = clock() - fileYomikomi - startTime;
//
//	std::cout << "end \"Yomikomi!\" " << std::endl;
//	i_max = i;
//	std::vector<std::string> startpos;
//	startpos.push_back(" ");
//	startpos.push_back("startpos");
//	Kyokumen kyo = Kyokumen(startpos);
//
//	node = test[0];
//	tree.setRoot(node, kyo, i_max);
//
//	endTime = clock() - startTime;
//
//	std::ofstream ofs("time.txt", std::ios::app);
//	ofs << "読み込みタイプ," << yomikomi_type << ",ノード数," << i_max << ",読み込み時間," << yomikomijikan / (double)CLOCKS_PER_SEC << ",復元時間," << hukugenjikan / (double)CLOCKS_PER_SEC << ",ファイル読み込み完了時刻," << fileYomikomi / (double)CLOCKS_PER_SEC << ",探索木読み込み完了時刻," << tansakugiYomikomi / (double)CLOCKS_PER_SEC << ",行読み時間," << getlinejikan / (double)CLOCKS_PER_SEC << ",総時間," << endTime / (double)CLOCKS_PER_SEC << "," << yomikomi_file_name << ".txt" << std::endl;
//	ofs.close();
//}
