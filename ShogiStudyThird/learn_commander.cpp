#include "learn_commander.h"
#include "usi.h"
#include <iostream>
#include <fstream>
#include <filesystem>

void LearnCommander::execute() {
	LearnCommander commander;
	while (!commander.quit) {
		std::string usiin;
		std::getline(std::cin, usiin);
		auto tokens = usi::split(usiin, ' ');
		if (tokens.empty()) {
			std::cout << "command ready" << std::endl;
		}
		else if (tokens[0] == "usi") {
			coutID();
			coutOption();
			coutLearnerOption();
			std::cout << "usiok" << std::endl;
		}
		else if (tokens[0] == "setoption") {
			commander.setOption(tokens);
			commander.setLearnerOption(tokens);
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
			commander.gameover(tokens);
		}
		else if (tokens[0] == "saveparam") {
			//更新したパラメータを保存
			//saveparam [保存先フォルダ]
			if (tokens.size() >= 2) Evaluator::save(usiin.substr(10));
			else Evaluator::save();
			std::cout << "saveparam done." << std::endl;
		}
		else if (tokens[0] == "genparam") {
			//学習前の初期パラメータを生成する
			//genparam 出力先フォルダ
			Evaluator::genFirstEvalFile(usiin.substr(9));
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
		else if (tokens[0] == "showBanFigure") {
			std::cout << commander.tree.getRootPlayer().kyokumen.toBanFigure() << std::endl;
		}
		else if (tokens[0] == "quit") {
			return;
		}
	}
}

void LearnCommander::coutID() {
	std::cout << "id name ";
#ifdef LEARN_TD_LAMBDA_PROB
	std::cout << "TD(l)prob_";
#endif
#ifdef LEARN_TD_LAMBDA_CP
	std::cout << "TD(l)cp_";
#endif
#ifdef LEARN_PG
	std::cout << "PG_";
#endif
#ifdef LEARN_REGRESSION
	std::cout << "Reg_";
#endif
#ifdef LEARN_BOOTSTRAP_ROOT
	std::cout << "BTSroot_";
#endif
#ifdef LEARN_BOOTSTRAP_RANDOM_NODE
	std::cout << "BTSnode_";
#endif
#ifdef SAMPLING_GRAD_PROB
	std::cout << "sampling(prob)_";
#endif
#ifdef SAMPLING_GRAD_CP
	std::cout << "sampling(cp)_";
#endif
#ifdef PVLEAF_GRAD
	std::cout << "pvleaf_";
#endif
	std::cout << "Learn" << std::endl;
	std::cout << "id author Hiromasa_Iwamoto" << std::endl;
}

void LearnCommander::coutLearnerOption() {
	using namespace std;
	cout << "option name Batch_Num type spin default 1 min 1 max 99999999999" << endl;
	cout << "option name T_search type string default 120" << endl;
#ifdef SAMPLING_GRAD_PROB
	cout << "option name Grad_Sampling_Num type spin default 10000 min 1 max 99999999999" << endl;
	cout << "option name T_sampling_grad_prob type string default 1.0" << endl;
#endif
#ifdef LEARN_TD_LAMBDA
	cout << "option name TD_lambda type string default 0.9" << endl;
	cout << "option name TD_gamma type string default 0.95" << endl;
	cout << "option name TD_rate type string default 0.1" << endl;
#endif
#ifdef LEARN_REGRESSION
	cout << "option name Regression_rate type string default 200" << endl;
#endif
#ifdef LEARN_PG
	cout << "option name PG_rate type string default 0.1" << endl;
#endif
#ifdef LEARN_BOOTSTRAP_ROOT
	cout << "option name BTSroot_rate type string default 0.001" << endl;
#endif
#ifdef LEARN_BOOTSTRAP_RANDOM_NODE
	cout << "option name BTSrandom_probability_rate type string default 0.01" << endl;
	cout << "option name BTSrandom_rate type string default 0.0001" << endl;
#endif
	cout << "option name reward_win type string default 1" << endl;
	cout << "option name reward_lose type string default -1" << endl;
	cout << "option name reward_draw type string default 0" << endl;
}

void LearnCommander::setLearnerOption(const std::vector<std::string>& token) {
	if (token[2] == "eval_folderpath") {
		//継承元のオプションだが、学習するので出力先を設定している
		const auto str = usi::combine(token.begin() + 4, token.end(), ' ');
		Evaluator::setpath_output(str);
	}
	else if (token[2] == "Batch_Num") {
		batch = std::stoi(token[4]);
	}
	else if (token[2] == "T_search") {
		T_search = std::stoull(token[4]);
	}
#ifdef SAMPLING_GRAD_PROB
	else if (token[2] == "Grad_Sampling_Num") {
		samplingnum = std::stoull(token[4]);
	}
	else if (token[2] == "T_sampling_grad_prob") {
		T_sgp = std::stod(token[4]);
	}
#endif
#ifdef LEARN_TD_LAMBDA
	else if (token[2] == "TD_lambda") {
		td_lambda = std::stod(token[4]);
	}
	else if (token[2] == "TD_gamma") {
		td_gamma = std::stod(token[4]);
	}
	else if (token[2] == "TD_rate") {
		td_rate = std::stod(token[4]);
	}
#endif
#ifdef LEARN_REGRESSION
	else if (token[2] == "Regression_rate") {
		reg_rate = std::stod(token[4]);
	}
#endif
#ifdef LEARN_PG
	else if (token[2] == "PG_rate") {
		pg_rate = std::stod(token[4]);
	}
#endif
#ifdef LEARN_BOOTSTRAP_ROOT
	else if (token[2] == "BTSroot_rate") {
		btsroot_rate = std::stod(token[4]);
	}
#endif
#ifdef LEARN_BOOTSTRAP_RANDOM_NODE
	else if (token[2] == "BTSrandom_probability_rate") {
		btsrand_exp = std::stod(token[4]);
	}
	else if (token[2] == "BTSrandom_rate") {
		btsrand_rate = std::stod(token[4]);
	}
#endif
	else if (token[2] == "reward_win") {
		reward_win = std::stod(token[4]);
	}
	else if (token[2] == "reward_lose") {
		reward_lose = std::stod(token[4]);
	}
	else if (token[2] == "reward_draw") {
		reward_draw = std::stod(token[4]);
	}
}

void LearnCommander::gameInit() {
	BBkiki::init();
	Evaluator::init();
	SearchTemperature::Ts_max = T_search;
	SearchTemperature::Ts_min = T_search;
	SearchTemperature::TsDistFuncCode = 0;
	SearchTemperature::TsNodeFuncCode = 0;
	agents.setup();
}

bool LearnCommander::loadTempInfo(int& learncount, std::string& datestr, const std::string& dir) {
	std::ifstream fs(dir + "/.cmlearninfo");
	if (fs) {
		std::getline(fs, datestr);

		std::string buff;

		std::getline(fs, buff);
		learncount = std::stoi(buff);

		return true;
	}
	else {
		return false;
	}
}

bool LearnCommander::saveTempInfo(const int& learncount, const std::string& datestr, const std::string& dir) {
	std::filesystem::create_directories(dir);
	std::ofstream fs(dir + "/.learninfo");
	if (fs.is_open()) {
		fs << datestr << "\n";
		fs << learncount << "\n";
		return true;
	}
	else {
		return false;
	}
}

bool LearnCommander::LoadTempVec(LearnVec& vec, const int vecnum, const std::string& dir) {
	const std::string filepath = dir + "/.learngrad" + std::to_string(vecnum);
	if (!std::filesystem::exists(filepath)) return false;
	vec.load(filepath);
	return true;
}

bool LearnCommander::SaveTempVec(LearnVec& vec, const int vecnum, const std::string& dir) {
	std::filesystem::create_directories(dir);
	vec.save(dir + "/.learngrad" + std::to_string(vecnum));
	return true;
}

void LearnCommander::go(const std::vector<std::string>& tokens) {
	const Kyokumen& kyokumen = tree.getRootPlayer().kyokumen;
	tree.evaluationcount = 0;
	agents.startSearch();
	TimeProperty tp(kyokumen.teban(), tokens);
	go_alive = false;
	if (go_thread.joinable()) go_thread.join();
	go_alive = true;
	if (tp.rule == TimeProperty::TimeRule::infinite) return;

	go_thread = std::thread([this, tp]() {
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
		std::this_thread::sleep_for(1ms);

		if (tp.rule == TimeProperty::TimeRule::fischer || tp.left > 100ms) {
			do {
				loopcounter++;
				constexpr auto sleeptime = 50ms;
				std::this_thread::sleep_for(sleeptime);
				const auto bestnode = root->getBestChild();
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
			agents.pauseSearch();
			agents.joinPause();
		}
		else {
			std::this_thread::sleep_for(tp.added);
			agents.pauseSearch();
			agents.joinPause();
			const auto bestnode = root->getBestChild();
			provisonalBestMove = bestnode;
			recentBestNode = bestnode;
		}

		if (provisonalBestMove == nullptr) provisonalBestMove = recentBestNode;

		learn_from_tree(tree.getRootPlayer(), root, provisonalBestMove);
		chakushu(provisonalBestMove);
		});
}

void LearnCommander::gameover(const std::vector<std::string>& tokens) {
	go_alive = false;
	info_alive = false;
	quit = true;
	agents.pauseSearch();

	if (tokens.size() >= 2 && !learned_gameover) {
		if (tokens[1] == "win") learn_at_gameover(MyGameResult::PlayerWin);
		else if (tokens[1] == "lose") learn_at_gameover(MyGameResult::PlayerLose);
		else if (tokens[1] == "draw") learn_at_gameover(MyGameResult::Draw);
	}
}

void LearnCommander::learn_from_tree(const SearchPlayer& kyokumen, const SearchNode* const root, const SearchNode* const bestmove) {
	if (root == nullptr) return;

	if (learned_gameover) { return; }
	if (std::abs(root->eval) >= SearchNode::getMateScoreBound()) {
		//詰んでいたら学習終了
		learned_gameover = true;
		if (root->eval > 0) {
			learn_at_gameover(MyGameResult::PlayerWin);
		}
		else {
			learn_at_gameover(MyGameResult::PlayerLose);
		}
		return;
	}

	LearnVec dv;
	double eval = 0;
#ifdef LEARN_BOOTSTRAP_RANDOM_NODE
#define Learn_BTS_random(vec, H, V, player) {if(btsrand_exp >= random.rand01()){ vec.addGrad(LearnUtil::EvalToProb(H) - LearnUtil::EvalToProb(V), player);} }
#else
#define	Learn_BTS_random(dw, c, player) ((void)0)
#endif
#ifdef PVLEAF_GRAD
#ifdef LEARN_V
	{
		const SearchNode* node = root;
		SearchPlayer player = kyokumen;
		int depth = 0;
		while (!node->isLeaf()) {
			//二手ずつ進める (相手番が末端になっていればその手前で終了)
			const SearchNode* const child = LearnUtil::choiceBestChild(node);
			if (child == nullptr || child->isLeaf()) break;
			const SearchNode* next = LearnUtil::choiceBestChild(child);
			if (next == nullptr) break;
			player.proceed(child->move);
			Learn_BTS_random(dw_btsrand, Evaluator::evaluate(player), child->eval, player);
			player.proceed(next->move);
			Learn_BTS_random(dw_btsrand, Evaluator::evaluate(player), next->eval, player);
			node = next;
			depth++;
		}
#ifdef LEARN_QS_LEAF
		player = LearnUtil::getQSBest(node, player);
#endif
		dv.addGrad(1, player);
		eval = Evaluator::evaluate(player);
		//std::cout << "(d:" << depth << ") ";
	}
#endif //LEARN_V
#ifdef LEARN_PG
	const double T_pg = T_search;
	LearnVec pg_e_vec;
	if (!root->children.empty() && !root->isLeaf()) {//PG-Leaf
		double Z, e_min;
		Z = LearnUtil::getChildrenZ(root, T_pg, e_min);
		if (Z != 0) {
			for (const auto& a : root->children) {
				if (a.isLeaf() || a.children.empty()) continue;
				SearchPlayer player = kyokumen;
				//一手進めて自手番にする(一手も進まない場合はスキップしている)
				const SearchNode* node = LearnUtil::choiceBestChild(&a);
				if (node == nullptr) continue;
				player.proceed(a.move);
				Learn_BTS_random(dw_btsrand, Evaluator::evaluate(player), a.eval, player);
				player.proceed(node->move);
				while (!node->isLeaf()) {
					//二手ずつ進める (相手番が末端になっていればその手前で終了)
					const SearchNode* const child = LearnUtil::choiceBestChild(node);
					if (child == nullptr || child->isLeaf()) break;
					const SearchNode* next = LearnUtil::choiceBestChild(child);
					if (next == nullptr) break;
					Learn_BTS_random(dw_btsrand, Evaluator::evaluate(player), node->eval, player);
					player.proceed(child->move);
					Learn_BTS_random(dw_btsrand, Evaluator::evaluate(player), child->eval, player);
					player.proceed(next->move);
					node = next;
				}
#ifdef LEARN_QS_LEAF
				player = LearnUtil::getQSBest(node, player);
#endif
				const double p = std::exp(-(a.eval - e_min) / T_pg) / Z;
				if (a.move == bestmove->move) {
					pg_e_vec.addGrad(1.0 - p, player);
				}
				else {
					pg_e_vec.addGrad(-p, player);
				}
			}
		}
	}
#endif //LEARN_PG
#endif //PVLEAF_GRAD

#ifdef SAMPLING_GRAD
#ifdef LEARN_PG
	LearnVec pg_dQt, pg_dPQ;
	long long pg_Qtcount = 0;
#endif
	eval = root->eval;
	if (!root->isLeaf()) {
		const double T = SearchTemperature::Te;
		for (int _sample = 0; _sample < samplingnum; _sample++) {
			SearchPlayer player = kyokumen;
			double c = 1;
			//始めの2手を計算
			const double V0 = root->eval;
			const SearchNode* const a = LearnUtil::choicePolicyRandomChild(root, T, random.rand01());
			if (a != nullptr && !a->children.empty() && !a->isLeaf()) {
				const double Q0 = -a->eval;
				const SearchNode* node = LearnUtil::choicePolicyRandomChild(a, T, random.rand01());
				if (node != nullptr) {
					player.proceed(a->move);
					Learn_BTS_random(dw_btsrand, Evaluator::evaluate(player), a->eval, player);
					player.proceed(node->move);
					//末端まで移動
					while (!node->isLeaf() && !node->children.empty()) {
						const auto child = LearnUtil::choicePolicyRandomChild(node, T, random.rand01());
						if (child == nullptr || child->isLeaf() || child->children.empty()) break;
						const auto next = LearnUtil::choicePolicyRandomChild(child, T, random.rand01());
						if (next == nullptr) break;

						const double V = node->eval;
						const double Q = -child->eval;
#ifdef SAMPLING_GRAD_CP
						c *= (Q - V) / T + 1.0;
#elif defined(SAMPLING_GRAD_PROB)
						c *= (LearnUtil::EvalToProb(Q) - LearnUtil::EvalToProb(V)) / T_sgp + 1.0;
#endif
						Learn_BTS_random(dw_btsrand, Evaluator::evaluate(player), node->eval, player);
						player.proceed(child->move);
						Learn_BTS_random(dw_btsrand, Evaluator::evaluate(player), child->eval, player);
						player.proceed(next->move);

						node = next;
					}
#ifdef LEARN_PG
					pg_dPQ.addGrad(c, player);
					if (a->move == bestmove->move) {
						pg_dQt.addGrad(c, player);
						pg_Qtcount++;
					}
#endif
#ifdef SAMPLING_GRAD_CP
					c *= (Q0 - V0) / T + 1.0; //根直下の係数の計算を最後にすることでPGLeafの計算も一緒にできる
#elif defined(SAMPLING_GRAD_PROB)
					c *= (LearnUtil::EvalToProb(Q0) - LearnUtil::EvalToProb(V0)) / T_sgp + 1.0;
#endif

				}
			}
			dv.addGrad(c, player);
		}
		dv *= (1.0 / samplingnum);
	}
	else {
		dv.addGrad(1, kyokumen);
	}
#endif

#ifdef LEARN_TD_LAMBDA
	{
#ifdef LEARN_TD_LAMBDA_CP
		const double td_eval = eval;
#elif defined(LEARN_TD_LAMBDA_PROB)
		const double td_eval = LearnUtil::EvalToProb(eval);
#endif
		if (!td_first) {
			const double delta = td_gamma * td_eval - td_Vt;
			dw_td += delta * td_e_vec;
		}
		else {
			td_first = false;
		}
		td_Vt = td_eval;
		td_e_vec *= td_gamma * td_lambda;
		td_e_vec += dv;
	}
#endif
#ifdef LEARN_REGRESSION
	{
		const double Pwin = LearnUtil::EvalToProb(eval);
		dw_reg_win += (1.0 - Pwin) * (1 / LearnUtil::probT) * (1 - Pwin) * Pwin * dv;
		dw_reg_lose += (0 - Pwin) * (1 / LearnUtil::probT) * (1 - Pwin) * Pwin * dv;
	}
#endif
#ifdef LEARN_PG
#ifdef SAMPLING_GRAD
	pg_dQt *= 1.0 / pg_Qtcount;
	pg_dPQ *= -1.0 / samplingnum;
	pg_dQt += pg_dPQ;
	dw_pg += pg_dQt;
#endif
#ifdef PVLEAF_GRAD
	dw_pg += (1.0 / T_pg) * pg_e_vec;
#endif
#endif
#ifdef LEARN_BOOTSTRAP_ROOT
	{
		const double H = Evaluator::evaluate(kyokumen);
		dw_btsroot += (LearnUtil::EvalToProb(H) - LearnUtil::EvalToProb(eval)) * dv;
	}
#endif
#undef Learn_BTS_random
}

void LearnCommander::learn_at_gameover(MyGameResult result) {
	LearnVec dw;
	const double length_revision = kifulength_mean / (tree.getHistory().size() + 1.0); //対局の長さで学習の重みが変わらないように補正する係数
	const std::string vec_tempfilepath = Evaluator::getpath_output() + "/.learngrad";
	if (std::filesystem::exists(vec_tempfilepath)) dw.load(vec_tempfilepath);
#ifdef LEARN_TD_LAMBDA
	{
		const double r = LearnUtil::ResultToReward(result, reward_win, reward_draw, reward_lose);
		const double delta = r + td_gamma * td_Vt - td_Vt;
		dw_td += delta * td_e_vec;
		dw += length_revision * td_rate * dw_td;
	}
#endif
#ifdef LEARN_REGRESSION
	if (result == MyGameResult::PlayerWin) {
		dw += length_revision * reg_rate * dw_reg_win;
	}
	else if (result == MyGameResult::PlayerLose) {
		dw += length_revision * reg_rate * dw_reg_lose;
	}
#endif
#ifdef LEARN_PG
	{
		const double r = LearnUtil::ResultToReward(result, reward_win, reward_draw, reward_lose);
		dw += length_revision * pg_rate * r * dw_pg;
	}
#endif
#ifdef LEARN_BOOTSTRAP_ROOT
	dw += length_revision * -btsroot_rate * dw_btsroot;
#endif
#ifdef LEARN_BOOTSTRAP_RANDOM_NODE
	dw += length_revision * -btsrand_rate * dw_btsrand;
#endif
	dw.updateEval();//評価値に反映
	Evaluator::save();//保存
	dw.save(vec_tempfilepath);
	std::cout << "info string learn end." << std::endl;
}