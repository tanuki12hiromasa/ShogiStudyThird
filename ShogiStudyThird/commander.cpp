#include "stdafx.h"
#include "commander.h"
#include "usi.h" 
#include <iostream>
#include <fstream>
#include <iomanip>

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
				commander.yomikomi();
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
		}
		else if (tokens[0] == "quit") {
			return;
		}
		else if (tokens[0] == "yomikomi") {
			//読み込みを実行
			commander.yomikomi();
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
	assert(agents.empty());
	assert(TsDistribution.size() == agentNum);
	for (int i = 0; i < agentNum; i++) {
		const double Ts = TsDistribution[i];
		agents.push_back(std::unique_ptr<SearchAgent>(new SearchAgent(tree, Ts, i)));
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
		std::lock_guard<std::mutex> lock(coutmtx);
		std::cout << "bestmove win" << std::endl;
		return;
	}
	else if (tree.getRoot()->eval < -SearchNode::getMateScoreBound()) {
		std::lock_guard<std::mutex> lock(coutmtx);
		std::cout << "bestmove resign" << std::endl;
		return;
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
			if (!yomikomi_on) {
				tree.makeNewTree(tokens);
				releaseAgentAndTree(prevRoot);
			}
			else {
				yomikomi_on = false;
			}
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

void Commander::yomikomi()
{
	std::string ss;
	int i = 0, j = 0;
	std::ifstream ifs;
	size_t i_max = 0;
	std::vector<SearchNode*> test;

	SearchNode* node = NULL;
	std::vector<int> parents = {};
	parents.push_back(-1);
	//bool oute;
	int index = 0;
	int	st = 0;
	double eval = 0.0;
	double mass = 0.0;
	Move move;
	std::cout << "start \"Yomikomi!\" " << std::endl;
	static int num = 0;
	ifs.open(yomikomi_file_name + ".txt");
	if (ifs.fail()) {
		std::cerr << yomikomi_file_name + ".txtが見つかりませんでした" << std::endl;
	}
	std::getline(ifs, ss); //sfen
	//std::string sfen = "position " + ss;
	//tree.makeNewTree(usi::split(sfen, ' '));

	while (1) {
		std::getline(ifs, ss);
		if (ifs.eof()) {
			break;//ファイルの終わりならブレイク
		}
		auto gyou = usi::split(ss, ',');
		index = std::stoi(gyou[0]);		//std::cout << "0 " << gyou[i][0] << std::endl;
		st = std::stoi(gyou[1]);			//std::cout << "1 " << gyou[i][1] << std::endl;
		//oute = std::stoi(gyou[2]);		//std::cout << "2 " << gyou[i][2] << std::endl;
		//move = Move(gyou[3], 0, oute);	//std::cout << "3 " << gyou[i][3] << std::endl;
		std::string sss = gyou[2].erase(0, 1);
		move = Move(sss, false);	//std::cout << "3 " << gyou[i][3] << std::endl;
		eval = std::stod(gyou[3]);		//std::cout << "4 " << gyou[i][4] << std::endl;
		mass = std::stod(gyou[4]);		//std::cout << "5 " << gyou[i][5] << std::endl;
		j = 6;
		while (1) {		//子ノードのインデックスが読み終わるまでループ
			if (gyou[j] == "]") break;//1列の終わりならブレイク 
			parents.push_back(index);	 //親のインデックスを要素として持つ
			j++;	//]のチェック用
		}

		if (index == 0) {//1つ目は親なし
			test.push_back(node->restoreNode(Move(), st, eval, mass));
		}
		else {
			test.push_back(node->restoreNode(move, st, eval, mass));
			test[parents[index]]->children.push_back(test[index]);
		}
		i++;
	}
	std::cout << "end \"Yomikomi!\" " << std::endl;
	i_max = i;
	std::vector<std::string> startpos;
	startpos.push_back(" ");
	startpos.push_back("startpos");
	Kyokumen kyo = Kyokumen(startpos);

	node = test[0];
	tree.setRoot(node, kyo, i_max);
}
