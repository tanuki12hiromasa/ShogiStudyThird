#include "stdafx.h"
#include "commander.h"
#include "usi.h" 
#include <iostream>
#include <iomanip>
#include <fstream>

void Commander::execute(const std::string& enginename) {
	Commander commander;
	while (true) {
		std::string usiin;
		std::getline(std::cin, usiin);
		auto tokens = usi::split(usiin, ' ');
		if (tokens.empty()) {
			std::cout << "command ready" << std::endl;
		}
		else if (tokens[0] == "usi") {
			std::cout << "id name " << enginename << "_" << Evaluator::name() << std::endl;
			std::cout << "id author Hiromasa_Iwamoto" << std::endl;
			coutOption();
			Joseki::coutOption();
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
		else if (tokens[0] == "position") {
			commander.go_alive = false;
			commander.info_enable = false;
			commander.position(tokens);
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
			commander.chakushu(commander.tree.getBestMove());
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
			commander.agents.pauseSearch();
		}
		else if (tokens[0] == "debugsetup") {
			auto setLeaveNodeCommand = usi::split("setoption name leave_branchNode value true", ' ');
			commander.setOption(setLeaveNodeCommand);
			commander.gameInit();
			std::cout << "readyok" << std::endl;
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
	agents.terminate();
	if (deleteThread.joinable())deleteThread.detach();
	if(go_thread.joinable()) go_thread.join();
	if(info_thread.joinable())info_thread.join();
}

void Commander::coutOption() {
	using namespace std;
	cout << "option name eval_folderpath type string default ./data/kppt" << endl;
	cout << "option name leave_branchNode type check default false" << endl;
	cout << "option name continuous_tree type check default true" << endl;
	cout << "option name NumOfAgent type spin default 12 min 1 max 128" << endl;
	cout << "option name Repetition_score type string default 0" << endl;
	cout << "option name leave_qsearchNode type check default false" << endl;
	cout << "option name QSearch_Use_RelativeDepth type check default false" << endl;
	cout << "option name QSearch_depth type string default 8" << endl;
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
	//cout << "option name NodeMaxNum type string default 100000000" << endl;
	cout << "option name DrawMoveNum type spin default 320 min 0 max 1000000" << endl;
	cout << "option name PV_functionCode type spin default 0 min 0 max 3" << endl;
	cout << "option name PV_const type string default 0" << endl;
	cout << "option name resign_matemoves type spin default 3 min 0 max 40" << endl;//投了する詰み手数
	cout << "option name quick_bm_time_lower type spin default 4000 min 1000 max 600000" << endl;//即指しの判定時間の下限
	cout << "option name standard_time_upper type spin default 20000 min 1000 max 6000000" << endl;//即指しの判定時間の上限
	cout << "option name overhead_time type spin default 200 min 0 max 10000" << endl;
	cout << "option name estimate_movesnum type spin default 120 min 0 max 10000" << endl;
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
		else if (token[2] == "eval_folderpath") {
			//評価関数のパラメータファイルの位置を指定する
			const auto path = usi::combine(token.begin() + 4, token.end(), ' ');
			Evaluator::setpath_input(path);
		}
		else if (token[2] == "NumOfAgent") {
			agents.setAgentNum(std::stoi(token[4]));
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
			SearchTemperature::Ts_min = std::stod(token[4]);
		}
		else if (token[2] == "Ts_max") {
			SearchTemperature::Ts_max = std::stod(token[4]);
		}
		else if (token[2] == "Ts_disperseFunc") {
			SearchTemperature::TsDistFuncCode = std::stoi(token[4]);
		}
		else if (token[2] == "Ts_funcParam") {
			SearchTemperature::TsNodeFuncConstant = (std::stod(token[4]));
		}
		else if (token[2] == "Ts_functionCode") {
			SearchTemperature::TsNodeFuncCode = (std::stoi(token[4]));
		}
		else if (token[2] == "T_eval") {
			SearchTemperature::Te = (std::stod(token[4]));
		}
		else if (token[2] == "T_depth") {
			SearchTemperature::Td = (std::stod(token[4]));
		}
		else if (token[2] == "Es_functionCode") {
			SearchNode::setEsFuncCode(std::stoi(token[4]));
		}
		else if (token[2] == "Es_funcParam") {
			SearchNode::setEsFuncParam(std::stod(token[4]));
		}
		else if (token[2] == "DrawMoveNum") {
			SearchAgent::setDrawMoveNum(std::stoi(token[4]));
		}
		else if (token[2] == "PV_functionCode") {
			SearchNode::setPVFuncCode(std::stoi(token[4]));
		}
		else if (token[2] == "PV_const") {
			SearchNode::setPVConst(std::stod(token[4]));
		}
		else if (token[2] == "resign_matemoves") {
			resign_border = std::stoi(token[4]);
		}
		else if (token[2] == "quick_bm_time_lower") {
			time_quickbm_lower = std::chrono::milliseconds(std::stoi(token[4]));
		}
		else if (token[2] == "standard_time_upper") {
			time_standard_upper = std::chrono::milliseconds(std::stoi(token[4]));
		}
		else if (token[2] == "overhead_time") {
			time_overhead = std::chrono::milliseconds(std::stoi(token[4]));
		}
		else if (token[2] == "estimate_movesnum") {
			estimate_movesnum = std::stoi(token[4]);
		}
		else {
			joseki.setOption(token);
		}
	}
}

void Commander::paramInit() {
	//usiによる設定前のデフォルト値

	SearchNode::setQSearchDepth(8);
	SearchNode::setMateScore(34000);
	SearchNode::setMateOneScore(20);
	SearchNode::setMateScoreBound(30000);
	SearchNode::setRepScore(0);
}

void Commander::gameInit() {
	BBkiki::init();
	Evaluator::init();
	tree.reset();
	agents.setup();
	info();

	joseki.readBook();
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
	else if (joseki.getJosekiOn() && joseki.getBestMoveFromJoseki(kyokumen.toSfen())) {
		return;
	}
	tree.evaluationcount = 0ull;
	info_prev_evcount = 0ull;
	info_prevtime = std::chrono::system_clock::now();
	agents.startSearch();
	TimeProperty tp(kyokumen.teban(), tokens);
	go_alive = false;
	if(go_thread.joinable()) go_thread.join();
	go_alive = true;
	if (tp.rule == TimeProperty::TimeRule::infinite) return;
	go_thread = std::thread([this,tp]() {
		using namespace std::chrono_literals;
		const auto starttime = std::chrono::system_clock::now();
		const SearchNode* root = tree.getRoot();
		const auto timelimit = decide_timelimit(tp);
		auto searchtime = timelimit.first;//探索時間
		SearchNode* provisonalBestMove = nullptr;//暫定着手
		double provisonal_pi = 0;//暫定着手の方策
		SearchNode* recentBestNode = nullptr;//直前の最善ノード
		double pi_average = 0;//最善手の方策の時間平均
		int continuous_counter = 0;//最善手が同じまま連続している回数
		int changecounter = 0;
		int loopcounter = 0;
		std::cout << "info string time:" << timelimit.first.count() << ", " << timelimit.second.count() << std::endl;
		std::this_thread::sleep_for(searchtime / 32);
		do {
			loopcounter++;
			constexpr auto sleeptime = 50ms;
			std::this_thread::sleep_for(sleeptime);
			const auto bestnode = root->getBestChild();
			if (bestnode != nullptr) {
				const double pi = root->getChildRate(bestnode, 40);
				if (bestnode == recentBestNode) { //最善ノードが変わっていない
					pi_average = (pi_average * continuous_counter + pi) / ((double)continuous_counter + 1);
					continuous_counter++;
					if (continuous_counter > 4) { //一定回数以上最善が不変であれば信頼できるとして暫定着手とする
						provisonalBestMove = recentBestNode;
						provisonal_pi = pi_average;
					}
				}
				else {
					changecounter++;
					pi_average = pi;
					continuous_counter = 1;
				}
				recentBestNode = bestnode;
			}
			//即指しの条件を満たしたら指す
			if (continuous_counter * sleeptime > std::max(timelimit.first / 2, time_quickbm_lower)) {
				break;
			}
			if ((loopcounter & 0xF) == 0) {
				double changerate = (double)changecounter / loopcounter;
				searchtime = (changerate > 0.05) ? std::chrono::duration_cast<std::chrono::milliseconds>(timelimit.first * changerate / 0.05) : timelimit.first;
			}
			//標準時間になったら指すか決める もし拮抗している局面なら時間を延長する
			if (std::chrono::system_clock::now() - starttime >= searchtime && provisonalBestMove != nullptr) {
				break;
			}
			//時間上限になったら指す
			if (std::chrono::system_clock::now() - starttime + sleeptime >= timelimit.second) {
				break;
			}
		} while (std::abs(root->eval) < SearchNode::getMateScoreBound());
		if (provisonalBestMove == nullptr) provisonalBestMove = recentBestNode;
		chakushu(provisonalBestMove);
	});
	info_enable = true;
}

//first:標準的な思考時間 second:思考時間の上限
std::pair<std::chrono::milliseconds, std::chrono::milliseconds> Commander::decide_timelimit(const TimeProperty time)const {
	using namespace std::chrono_literals;
	switch (time.rule) {
		case TimeProperty::TimeRule::byoyomi: {
			const auto standerd_time = std::min(std::max(time.left / std::max(estimate_movesnum - tree.getMoveNum(), 5), time.added), time_standard_upper) - time_overhead;
			const auto limit_time = time.left + time.added - time_overhead;
			return std::make_pair(standerd_time, limit_time);
		}
		case TimeProperty::TimeRule::fischer: {
			const int expected_movesleft = std::max(estimate_movesnum - tree.getMoveNum(), 2);
			const auto standerd_time = std::min(time.left / expected_movesleft + time.added, time_standard_upper) - time_overhead;
			const auto limit_time = time.left + time.added - time_overhead;
			return std::make_pair(standerd_time, limit_time);
		}
		default:
			assert(0);
			return std::make_pair(5s, 5s);
	}
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
				const uint64_t evcount = tree.getEvaluationCount();
				const auto now = std::chrono::system_clock::now();
				if (info_enable) {
					const int nps = (evcount - info_prev_evcount) / ((double)std::chrono::duration_cast<std::chrono::milliseconds>(now - info_prevtime).count() / 1000);
					info_prevtime = now;
					//std::cout << "info string info" << std::endl;
					const auto PV = tree.getPV();
					std::string pvstr;
					if (PV.size() >= 2) {
						for (int i = 1; i < 15 && i < PV.size() && PV[i] != nullptr; i++) pvstr += PV[i]->move.toUSI()+' ';
						const auto& root = PV[0];
						std::cout << std::fixed;
						std::cout << "info pv " << pvstr << "depth " << std::setprecision(2) << root->mass << " seldepth " << (PV.size()-1)
							<< " score cp " << static_cast<int>(root->eval) << " nodes " << SearchNode::getNodeCount() << " nps " << nps << std::endl;
					}
					else {
						std::cout << "info string failed to get pv" << std::endl;
					}
				}
				info_prev_evcount = evcount;
			}
		});
	}
}

void Commander::chakushu(SearchNode* const bestchild) {
	std::lock_guard<std::mutex> clock(coutmtx);
	std::lock_guard<std::mutex> tlock(treemtx);
	agents.pauseSearch();
	info_enable = false;
	const Kyokumen& kyokumen = tree.getRootPlayer().kyokumen;
	if (kyokumen.isDeclarable()) {
		std::cout << "bestmove win" << std::endl;
		return;
	}
	SearchNode* const root = tree.getRoot();
	if (root->eval < -SearchNode::getMateScoreBound() && root->getMateNum() >= -resign_border) {
		std::cout << "info score cp " << static_cast<int>(root->eval) << std::endl;
		std::cout << "bestmove resign" << std::endl;
		return;
	}
	if (bestchild == nullptr) {
		//std::cout << "info string error no children" << std::endl;
		std::cout << "bestmove resign" << std::endl;
		return;
	}
	std::string pvstr;
	int depth = 1;
	for (SearchNode* node = bestchild; depth < 15 && !node->isLeaf() && node != nullptr; depth++,node = node->getBestChild()) pvstr += node->move.toUSI() + ' ';
	std::cout << std::fixed;
	std::cout << "info pv " << pvstr << "depth " << std::setprecision(2) << root->mass << " seldepth " << depth
		<< " score cp " << static_cast<int>(root->eval) << " nodes " << SearchNode::getNodeCount() << std::endl;
	std::cout << "bestmove " << bestchild->move.toUSI() << std::endl;
	tree.proceed(bestchild);
	agents.noticeProceed();
	if (permitPonder) {
		agents.startSearch();
	}
	return;
}

void Commander::position(const std::vector<std::string>& tokens) {
	std::lock_guard<std::mutex> lock(treemtx);
	agents.pauseSearch();
	//agents.joinPause(); //agentの探索が停止するのを待ってから次に進めたい場合はこの処理を行わせる バグ等がなければ待たなくても大丈夫な筈
	tree.set(tokens);
	agents.noticeProceed();
}
