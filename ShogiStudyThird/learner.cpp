#include "learner.h"
#include <iostream>
#include <fstream>
#include "usi.h"

void Learner::execute() {
	Learner learner;
	LearnVec dw;
	while (true) {
		std::string usiin;
		std::getline(std::cin, usiin);
		auto tokens = usi::split(usiin, ' ');
		if (tokens.empty()) {
			std::cout << "command ready" << std::endl;
		}
		else if (tokens[0] == "setoption") {
			if (tokens[1] == "evalinput") {
				Evaluator::setpath_input(tokens[2]);
				std::cout << "set evalinput" << std::endl;
			}
			else if (tokens[1] == "evaloutput") {
				Evaluator::setpath_output(tokens[2]);
				std::cout << "set evaloutput" << std::endl;
			}
		}
		else if (tokens[0] == "init") {
			//設定ファイルを読み込む
			//init 設定ファイル.txt
			learner.init(tokens);
			std::cout << "init done." << std::endl;
		}
		else if (tokens[0] == "genparam") {
			//学習前の初期パラメータを生成する
			//genparam 出力先フォルダ
			Evaluator::genInitialParam(usiin.substr(9));
		}
		else if (tokens[0] == "saveparam") {
			//更新したパラメータを保存
			//saveparam [保存先フォルダ]
			if (tokens.size() > 2) Evaluator::setpath_input(usiin.substr(10));
			dw.updateEval();
			Evaluator::save();
			std::cout << "saveparam done." << std::endl;
		}
		else if (tokens[0] == "loadgrad") {
			dw.load(tokens.size() > 1 ? tokens[1] : "learninggrad.bin");
		}
		else if (tokens[0] == "savegrad") {
			dw.save(tokens.size() > 1 ? tokens[1] : "learninggrad.bin");
		}
		else if (tokens[0] == "selfplaylearn") {
			//自己対局を行い、その棋譜データで学習を行うことを繰り返す
			if (tokens.size() > 2) {
				if (tokens[2] == "bts0") {
					//単純なbootstrapのみで学習
					learner.selfplay_rootstrap(dw);
				}
				else if (tokens[2] == "btspv") {
					learner.selfplay_bootstrap(dw);
				}
				else if (tokens[2] == "bts1") {
					learner.selfplay_child_bootstrap(dw);
				}
				else if (tokens[2] == "reg") {
					learner.selfplay_sampling_regression(dw);
				}
				else if (tokens[2] == "pge") {
					learner.selfplay_sampling_pge(dw);
				}
				else if (tokens[2] == "td") {
					learner.selfplay_sampling_td(dw);
				}
				else if (tokens[2] == "btss") {
					const int samplingnum = (tokens.size() > 3) ? std::stoi(tokens[3]) : 20;
					learner.selfplay_sampling_bts(samplingnum);
				}
			}
		}
		else if (tokens[0] == "quit") {
			break;
		}
	}
}

void Learner::init(const std::vector<std::string>& cmdtokens) {
	BBkiki::init();
	Evaluator::init();
}

void Learner::search(SearchTree& tree) {
	search(tree, searchtime);
}

void Learner::search(SearchTree& tree, const std::chrono::milliseconds time) {
	using namespace std::chrono_literals;
	constexpr auto checkflame = 50ms;
	std::vector<std::unique_ptr<SearchAgent>> agents;
	for (int i = 0; i < agentnum; i++) {
		agents.push_back(std::unique_ptr<SearchAgent>(new SearchAgent(tree, T_search, i)));
	}
	const auto starttime = std::chrono::system_clock::now();
	const SearchNode* const root = tree.getRoot();
	while (std::abs(root->eval) < SearchNode::getMateScoreBound() && std::chrono::system_clock::now() - starttime < time) {
		std::this_thread::sleep_for(checkflame);
	}
	for (auto& ag : agents) {
		ag->stop();
	}
	for (auto& ag : agents) {
		ag->terminate();
	}
	agents.clear();
}

//どちらが勝ったかを返す関数 1:先手勝ち -1:後手勝ち 0:引き分け
int Learner::getWinner(std::vector<std::string>& sfen) {
	int startnum = 0;
	for (int t = 0; t < sfen.size(); t++) {
		if (sfen[t] == "moves") {
			startnum = t;
			break;
		}
	}
	if (sfen.back() == "resign") {
		sfen.pop_back();
		return ((sfen.size() - startnum) % 2 != 0) ? 1 : -1;
	}
	else return 0;
}


LearnVec Learner::simple_bootstrap(const Kyokumen startKyokumen, const std::vector<Move>& kifu, const int winner, const bool learnteban) {
	std::vector<Move> history;
	if (!learnteban)history.push_back(kifu[0]);
	int kifuLength = kifu.size();
	std::cout << "L=" << (kifuLength - 1) << std::endl;
	const double Tb = SearchNode::getTeval();
	//tree初期化
	SearchTree tree;
	tree.makeNewTree(startKyokumen, {});
	LearnVec dw;
	std::cout << "reinforcement learning " << std::endl;
	std::cout << "t=" << ((learnteban) ? 0 : 1) << ":";
	search(tree);
	std::cout << " searched.";
	for (int t = (learnteban) ? 0 : 1; t < kifuLength - 1; t = t + 2) {
		const auto root = tree.getRoot();
		double Pwin = LearnUtil::EvalToProb(root->eval);
		LearnVec rootVec;
		rootVec.addGrad(1, tree.getRootPlayer());
		//bts
		if (learning_rate_bts > 0) {
			dw += -learning_rate_bts * (LearnUtil::EvalToProb(root->getOriginEval()) - Pwin) * rootVec;
		}
		std::cout << " calculated." << std::endl;

		//proceed
		history.push_back(kifu[t]);
		history.push_back(kifu[t + 1]);
		tree.clear();
		tree.makeNewTree(startKyokumen, history);
		std::cout << "t=" << (t + 1) << ":";
		search(tree);
		std::cout << " searched.";
	}

	//ノード消去
	tree.clear();
	std::cout << "reinforcement learning finished." << std::endl;
	return dw;
}

LearnVec Learner::sampling_bootstrap(const Kyokumen startKyokumen, const std::vector<Move>& kifu, const int winner, const bool learnteban) {
	std::vector<Move> history;
	if (!learnteban)history.push_back(kifu[0]);
	int kifuLength = kifu.size();
	std::cout << "L=" << (kifuLength - 1) << std::endl;
	const double Tb = SearchNode::getTeval();
	//tree初期化
	SearchTree tree;
	tree.makeNewTree(startKyokumen, {});
	LearnVec dw;
	std::cout << "reinforcement learning " << std::endl;
	std::cout << "t=" << ((learnteban) ? 0 : 1) << ":";
	search(tree);
	std::cout << " searched.";
	for (int t = (learnteban) ? 0 : 1; t < kifuLength - 1; t = t + 2) {
		const auto root = tree.getRoot();
		double Pwin = LearnUtil::EvalToProb(root->eval);
		LearnVec rootVec = LearnUtil::getGrad(root, tree.getRootPlayer(), learnteban, 1000, 0);
		//bts
		if (learning_rate_bts > 0) {
			dw += -learning_rate_bts * (LearnUtil::EvalToProb(root->getOriginEval()) - Pwin) * rootVec;
		}
		std::cout << " calculated." << std::endl;

		//proceed
		history.push_back(kifu[t]);
		history.push_back(kifu[t + 1]);
		tree.clear();
		tree.makeNewTree(startKyokumen, history);
		std::cout << "t=" << (t + 1) << ":";
		search(tree);
		std::cout << " searched.";
	}

	//ノード消去
	tree.clear();
	std::cout << "reinforcement learning finished." << std::endl;
	return dw;
}


void Learner::selfplay_rootstrap(LearnVec& dw) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };

	std::cout << "self-play rootstrap learning \n";
	std::vector<Move> history;
	const Kyokumen startpos;
	int winner = 0;
	{
		SearchTree tree;
		tree.makeNewTree(startpos, {});
		while (true) {
			search(tree, searchtime);
			const auto root = tree.getRoot();
			if (root->eval >= SearchNode::getMateScoreBound()) {
				winner = tree.getRootPlayer().kyokumen.teban() ? 1 : -1;
				history.push_back(LearnUtil::choiceBestChild(root)->move);
				break;
			}
			else if (root->eval <= -SearchNode::getMateScoreBound()) {
				winner = tree.getRootPlayer().kyokumen.teban() ? -1 : 1;
				break;
			}

			//bts
			const auto& rootplayer = tree.getRootPlayer();
			LearnVec rootVec;
			const double sigH = LearnUtil::EvalToProb(Evaluator::evaluate(rootplayer));
			double c = LearnUtil::probT * sigH * (1 - sigH);
			rootVec.addGrad(c, rootplayer);
			if (learning_rate_bts > 0) {
				dw += -learning_rate_bts * (sigH - LearnUtil::EvalToProb(root->eval)) * rootVec;
			}

			const auto next = LearnUtil::choiceChildRandom(root, T_selfplay, random(engine));
			tree.proceed(next);
			history.push_back(next->move);
			std::cout << next->move.toUSI() << std::endl;
		}
		std::cout << "gameend." << std::endl;
	}
	std::cout << "self-play learning finished" << std::endl;
}

void Learner::selfplay_bootstrap(LearnVec& dw) {
	SearchNode::setEsFuncCode(0);
	std::cout << "self-play bootstrap learning \n";
	std::vector<Move> history;
	const Kyokumen startpos;
	{
		SearchTree tree;
		while (true) {
			tree.makeNewTree(startpos, history);
			search(tree, searchtime);
			const auto root = tree.getRoot();
			if (std::abs(root->eval) >= SearchNode::getMateScoreBound()) {
				const auto& player = tree.getRootPlayer();
				const double sigH = LearnUtil::EvalToProb(Evaluator::evaluate(player));
				const double c = -learning_rate_bts * (sigH - LearnUtil::EvalToProb(root->eval))
					* LearnUtil::probT * sigH * (1 - sigH);
				dw.addGrad(c, player);
				break;
			}
			//bts
			if (learning_rate_bts > 0) {
				auto player = tree.getRootPlayer();
				SearchNode* node = root;
				while (!node->isLeaf()) {
					const double sigH = LearnUtil::EvalToProb(Evaluator::evaluate(player));
					const double c = -learning_rate_bts * (sigH - LearnUtil::EvalToProb(root->eval))
						* LearnUtil::probT * sigH * (1 - sigH);
					dw.addGrad(c, player);
					node = LearnUtil::choiceBestChild(node);
					if (node->children.empty()) break;
					player.proceed(node->move);
				}
			}
			const auto next = LearnUtil::choiceBestChild(root);
			history.push_back(next->move);
			std::cout << next->move.toUSI() << "(" << next->eval << ")" << std::endl;
			tree.clear();
		}
		std::cout << "gameend." << std::endl;
	}
	std::cout << "self-play learning finished" << std::endl;
}

void Learner::selfplay_child_bootstrap(LearnVec& dw) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };

	std::cout << "self-play bts-c learning \n";
	std::vector<Move> history;
	const Kyokumen startpos;
	int winner = 0;
	{
		SearchTree tree;
		tree.makeNewTree(startpos, {});
		const double T = T_selfplay;
		while (true) {
			search(tree, searchtime);
			const auto root = tree.getRoot();
			if (root->eval >= SearchNode::getMateScoreBound()) {
				winner = tree.getRootPlayer().kyokumen.teban() ? 1 : -1;
				history.push_back(LearnUtil::choiceBestChild(root)->move);
				break;
			}
			else if (root->eval <= -SearchNode::getMateScoreBound()) {
				winner = tree.getRootPlayer().kyokumen.teban() ? -1 : 1;
				break;
			}

			//bts
			if (learning_rate_bts > 0) {
				const auto& rootplayer = tree.getRootPlayer();
				LearnVec rootVec;
				double cmin = std::numeric_limits<double>::max();
				for (const auto child : root->children) {
					if (child->eval < cmin) {
						cmin = child->eval;
					}
				}
				double Z = 0;
				for (const auto child : root->children) {
					Z += std::exp(-(child->eval - cmin) / T);
				}
				for (const auto child : root->children) {
					auto cplayer = rootplayer;
					const double pi = std::exp(-(child->eval - cmin) / T) / Z;
					cplayer.proceed(child->move);
					const double c_sigE = LearnUtil::EvalToProb(child->eval);
					const double c_sigH = LearnUtil::EvalToProb(Evaluator::evaluate(cplayer));
					const double c = -(c_sigH - c_sigE) * LearnUtil::probT * c_sigH * (1 - c_sigH);
					rootVec.addGrad(c * pi, cplayer);
					//std::cout << child->move.toUSI() << ":" << c_sigH << "," << c_sigE << " ";
				}
				dw += learning_rate_pp * rootVec;
			}

			const auto next = LearnUtil::choiceChildRandom(root, T_selfplay, random(engine));
			tree.proceed(next);
			history.push_back(next->move);
			std::cout << next->move.toUSI() << "(" << next->eval << ")" << std::endl;
		}
		std::cout << "gameend." << std::endl;
	}
	std::cout << "self-play learning finished" << std::endl;
}

void Learner::selfplay_sampling_regression(LearnVec& dw) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };

	LearnVec dw_sWin;
	LearnVec dw_gWin;
	std::cout << "self-play sampling regression learning \n";
	std::vector<Move> history;
	const Kyokumen startpos;
	bool sentewin = true;
	{
		SearchTree tree;
		tree.makeNewTree(startpos, {});
		while (true) {
			search(tree, searchtime);
			const auto root = tree.getRoot();
			if (root->eval >= SearchNode::getMateScoreBound()) {
				sentewin = tree.getRootPlayer().kyokumen.teban();
				history.push_back(LearnUtil::choiceBestChild(root)->move);
				break;
			}
			else if (root->eval <= -SearchNode::getMateScoreBound()) {
				sentewin = !tree.getRootPlayer().kyokumen.teban();
				break;
			}

			//reg
			const auto& rootplayer = tree.getRootPlayer();
			bool rootteban = rootplayer.kyokumen.teban();
			LearnVec Pwin_grad = LearnUtil::getSamplingGradV(root, rootplayer, 100);
			double Pwin = LearnUtil::EvalToProb(root->eval);
			//Pwin_grad *= Pwin * (1 - Pwin) * LearnUtil::probT;
			if (rootteban) {
				dw_sWin += (1 - Pwin) * Pwin_grad;
				dw_gWin += (0 - Pwin) * Pwin_grad;
			}
			else {
				dw_gWin += (1 - Pwin) * Pwin_grad;
				dw_sWin += (0 - Pwin) * Pwin_grad;
			}

			const auto next = LearnUtil::choiceChildRandom(root, T_selfplay, random(engine));
			tree.proceed(next);
			history.push_back(next->move);
			//tree.deleteBranch(root, history);
			std::cout << next->move.toUSI() << "(" << next->eval << ")" << std::endl;
		}
		std::cout << "gameend." << std::endl;
	}

	if (sentewin) {
		dw_sWin *= learning_rate_reg;
		dw += dw_sWin;
	}
	else {
		dw_gWin *= learning_rate_reg;
		dw += dw_gWin;
	}
	std::cout << "self-play learning finished" << std::endl;
}

void Learner::selfplay_sampling_pge(LearnVec& dw) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	const double Ta = T_search;

	std::cout << "self-play sampling pge learning \n";
	std::vector<Move> history;
	const Kyokumen startpos;
	SearchNode::setTeval(T_search);
	SearchNode::setTdepth(T_search);
	int winner = 0;
	{
		SearchTree tree;
		tree.makeNewTree(startpos, {});
		while (true) {
			search(tree, searchtime);
			const auto root = tree.getRoot();
			if (root->eval >= SearchNode::getMateScoreBound()) {
				winner = tree.getRootPlayer().kyokumen.teban() ? 1 : -1;
				history.push_back(LearnUtil::choiceBestChild(root)->move);
				break;
			}
			else if (root->eval <= -SearchNode::getMateScoreBound()) {
				winner = tree.getRootPlayer().kyokumen.teban() ? -1 : 1;
				break;
			}

			//pge
			if (learning_rate_pge > 0) {
				const auto& rootplayer = tree.getRootPlayer();
				LearnVec vec;
				double min = std::numeric_limits<double>::max();
				Move bestmove = root->children.front()->move;
				for (const auto child : root->children) {
					if (child->eval < min) {
						min = child->eval;
						bestmove = child->move;
					}
				}
				double Z = 0;
				for (const auto child : root->children) {
					Z += std::exp(-(child->eval - min) / Ta);
				}
				for (const auto child : root->children) {
					const double pi = std::exp(-(child->eval - min) / Ta) / Z;
					//std::cout << pi << " " << std::endl;
					auto cplayer = rootplayer;
					cplayer.proceed(child->move);
					LearnVec childvec = LearnUtil::getSamplingGradQ(child, cplayer, 1000 * pi);
					if (child->move == bestmove) {
						vec += (1.0 - pi) * childvec;
					}
					else {
						vec += (-pi) * childvec;
					}
				}
				dw += learning_rate_pge / LearnUtil::pTb * vec;
			}

			const auto next = LearnUtil::choiceChildRandom(root, T_search, random(engine));
			tree.proceed(next);
			history.push_back(next->move);
			//tree.deleteBranch(root, history);
			std::cout << next->move.toUSI() << "(" << next->eval << ")" << std::endl;
		}
		std::cout << "gameend." << std::endl;
	}

	std::cout << "self-play learning finished" << std::endl;
}

void Learner::selfplay_sampling_td(LearnVec& dw) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	SearchNode::setTeval(T_selfplay);

	LearnVec tdvec;
	LearnVec s_e_lambda;
	double s_V_t = 0;
	LearnVec g_e_lambda;
	double g_V_t = 0;
	std::cout << "self-play sampling td-lamda learning \n";
	std::vector<Move> history;
	const Kyokumen startpos;
	bool sentewin = true;
	{
		SearchTree tree;
		tree.makeNewTree(startpos, {});
		while (true) {
			search(tree, searchtime);
			const auto root = tree.getRoot();

			if (root->eval >= SearchNode::getMateScoreBound()) {
				sentewin = tree.getRootPlayer().kyokumen.teban();
				history.push_back(LearnUtil::choiceBestChild(root)->move);
				break;
			}
			else if (root->eval <= -SearchNode::getMateScoreBound()) {
				sentewin = !tree.getRootPlayer().kyokumen.teban();
				break;
			}

			//td-lambda
			if (tree.getRootPlayer().kyokumen.teban()) {
				double sigV = LearnUtil::EvalToProb(root->eval);
				if (history.size() >= 2) {
					tdvec += (td_gamma * sigV - s_V_t) * s_e_lambda;
				}
				s_e_lambda *= td_gamma * td_lambda;
				s_e_lambda += LearnUtil::getSamplingGradV(root, tree.getRootPlayer(), 100);
				s_V_t = sigV;
			}
			else {
				double sigV = LearnUtil::EvalToProb(root->eval);
				if (history.size() >= 2) {
					tdvec += (td_gamma * sigV - g_V_t) * g_e_lambda;
				}
				g_e_lambda *= td_gamma * td_lambda;
				g_e_lambda += LearnUtil::getSamplingGradV(root, tree.getRootPlayer(), 100);
				g_V_t = sigV;
			}

			const auto next = LearnUtil::choiceChildRandom(root, T_selfplay, random(engine));
			tree.proceed(next);
			history.push_back(next->move);
			//tree.deleteBranch(root, history);
			std::cout << next->move.toUSI() << "(" << next->eval << ")" << std::endl;
		}
		const double s_V_t1 = LearnUtil::EvalToProb(tree.getRootPlayer().kyokumen.teban() ? Evaluator::evaluate(tree.getRootPlayer()) : -Evaluator::evaluate(tree.getRootPlayer()));
		const double g_V_t1 = 1 - s_V_t1;
		const double s_delta_t = (sentewin ? 1 : 0) + td_gamma * s_V_t1 - s_V_t;
		const double g_delta_t = (sentewin ? 0 : 1) + td_gamma * g_V_t1 - g_V_t;
		tdvec += s_delta_t * s_e_lambda;
		tdvec += g_delta_t * g_e_lambda;
		std::cout << "gameend." << std::endl;
	}
	tdvec *= learning_rate_td;
	dw += tdvec;
	std::cout << "self-play learning finished" << std::endl;
}

void Learner::selfplay_sampling_bts(const int samplingnum, double droprate) {
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };

	LearnVec dw;
	std::cout << "self-play bts-s learning \n";
	std::vector<Move> history;
	const Kyokumen startpos;
	int winner = 0;
	{
		SearchTree tree;
		tree.makeNewTree(startpos, {});
		const double T = T_selfplay;
		while (true) {
			search(tree, searchtime);
			const auto root = tree.getRoot();
			if (root->eval >= SearchNode::getMateScoreBound()) {
				winner = tree.getRootPlayer().kyokumen.teban() ? 1 : -1;
				history.push_back(LearnUtil::choiceBestChild(root)->move);
				break;
			}
			else if (root->eval <= -SearchNode::getMateScoreBound()) {
				winner = tree.getRootPlayer().kyokumen.teban() ? -1 : 1;
				break;
			}

			//bts
			if (learning_rate_bts > 0) {
				LearnVec vec;
				for (int samplingcount = 0; samplingcount < samplingnum; samplingcount++) {
					SearchNode* node = tree.getRoot();
					auto player = tree.getRootPlayer();
					while (node && !node->children.empty()) {
						const double sigE = LearnUtil::EvalToProb(node->eval);
						const double sigH = LearnUtil::EvalToProb(Evaluator::evaluate(player));
						const double c = -(sigH - sigE) * LearnUtil::probT * sigH * (1 - sigH);
						vec.addGrad(c / node->mass, player);
						node = LearnUtil::choiceChildRandom(node, T, random(engine));
						if (!node)break;
						player.proceed(node->move);
					}
				}
				dw += (learning_rate_bts_sampling / samplingnum) * vec;
			}

			const auto next = LearnUtil::choiceChildRandom(root, T_selfplay, random(engine));
			tree.proceed(next);
			history.push_back(next->move);
			std::cout << next->move.toUSI() << "(" << next->eval << ")" << std::endl;
		}
		std::cout << "gameend." << std::endl;
	}
	dw.clamp(1000);
	LearnVec::EvalClamp(30000);
	dw.updateEval();
	Evaluator::save();
	std::cout << "self-play learning finished" << std::endl;
}

