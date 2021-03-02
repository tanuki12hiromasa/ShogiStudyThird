#include "learner.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <string>
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
			Evaluator::genFirstEvalFile(usiin.substr(9));
		}
		else if (tokens[0] == "saveparam") {
			//更新したパラメータを保存
			//saveparam [保存先フォルダ]
			if (tokens.size() > 2) Evaluator::setpath_input(usiin.substr(10));
			dw.updateEval();
			Evaluator::save();
			std::cout << "saveparam done." << std::endl;
		}
		else if (tokens[0] == "printparam") {
			//Evaluator::print((tokens.size() > 1) ? std::stoi(tokens[1]) : 0);
		}
		else if (tokens[0] == "loadgrad") {
			dw.load(tokens.size() > 1 ? tokens[1] : "learninggrad.bin");
		}
		else if (tokens[0] == "savegrad") {
			dw.save(tokens.size() > 1 ? tokens[1] : "learninggrad.bin");
		}
		else if (tokens[0] == "randomposlearn") {
			if (tokens.size() < 2) { std::cout << "randomposlearn <batchsize> <itrsize>" << std::endl; continue; }
			learner.learn_start_by_randompos(std::stoi(tokens[1]), std::stoi(tokens[2]));
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

const std::string getDateString() {
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::string s(30, '\0');
	std::tm ima;
	localtime_s(&ima, &now);
	std::strftime(&s[0], s.size(), "%Y%m%d-%H%M", &ima);
	s.resize(s.find('\0'));
	return s;
}

void Learner::learn_start_by_randompos(const int batch, const int itr) {
	std::cout << "start randompos rootstarp learn\n";
	std::uniform_real_distribution<double> random{ 0, 1.0 };
	std::mt19937_64 engine{ std::random_device()() };
	const std::string tempinfo = "./.learninfo";
	const std::string tempgrad = "./.learngradient";
	std::string datestring = getDateString();
	int counter_itr = 0, counter_batch = 0;
	unsigned long long rootnum = 0, learnedposnum = 0;
	const double learn_rate_0 = 0.01;
	const double learn_rate_r = 0.5;
	LearnVec dw;
	//保険セーブが残っているなら再開
	{
		std::ifstream fs(tempinfo);
		if (fs) {
			std::getline(fs, datestring);
			std::string buff;
			std::getline(fs, buff);
			counter_itr = std::stoi(buff);
			std::getline(fs, buff);
			counter_batch = std::stoi(buff) + 1;
			if (counter_batch >= batch) { counter_itr++; counter_batch = 0; }
			std::getline(fs, buff);
			rootnum = std::stoull(buff);
			std::getline(fs, buff);
			learnedposnum = std::stoull(buff);
		}
	}
	const std::string learnlogdir = "./data/learnlog/" + datestring;
	std::filesystem::create_directories(learnlogdir);
	SearchTree tree;
	for (; counter_itr < itr; counter_itr++) {
		for (; counter_batch < batch; counter_batch++) {
			std::cout << "(" << counter_itr << "," << counter_batch << ")\n";
			LearnMethod* method = new SamplingBTS(dw, 0.001, 10000, 120);
			tree.makeNewTree(usi::split("position startpos", ' '));
			const int movesnum = 6 + 10 * random(engine);
			//ランダム局面を生成
			for (int i = 0; i < movesnum; i++) {
				const auto root = tree.getRoot();
				const auto& player = tree.getRootPlayer();
				const auto moves = MoveGenerator::genMove(root->move, player.kyokumen);
				if (moves.empty()) { counter_batch--; goto delTree; }
				root->addChildren(moves);
				const int randomchild = moves.size() * random(engine);
				tree.proceed(root->children.begin() + randomchild);
			}
			//rootが詰みの場合やり直し
			{
				const auto root = tree.getRoot();
				const auto& player = tree.getRootPlayer();
				const auto moves = MoveGenerator::genMove(root->move, player.kyokumen);
				if (moves.empty()) { counter_batch--; goto delTree; }
			}
			while (tree.getRoot() != nullptr) {
				//探索
				const auto root = tree.getRoot();
				const auto& rootplayer = tree.getRootPlayer();
				std::cout << root->move.toUSI() << " [" << rootplayer.kyokumen.toSfen() << "] ";
				search(tree);
				rootnum++;

				//ゲームが終了しているか調べる
				const auto pl = LearnUtil::getPrincipalLeaf(root);
				if (pl == nullptr) {
					break;
				}
				if (pl->isRepetition()) {
					method->fin(root, rootplayer, GameResult::Draw);
					break;
				}
				if (pl->isTerminal()) {
					method->fin(root, rootplayer, GameResult::SenteWin);//resultには結果を入れる (要実装)
					break;
				}

				//学習
				method->update(root, rootplayer);

				const auto bestchild = LearnUtil::choiceBestChild(root);
				tree.proceed(bestchild);
			}
			learnedposnum += method->getSamplingPosNum();
			//保険セーブ
		delTree:
			{
				dw.save(tempgrad);
				std::ofstream fs(tempinfo);
				if (fs) {
					fs << datestring << "\n" << counter_itr << "\n" << counter_batch << "\n" << rootnum << "\n" << learnedposnum << "\n";
				}
			}
			delete method;
			std::cout << "\n";
		}
		//評価関数に勾配を反映
		dw.updateEval();
		Evaluator::save();
		if ((((counter_itr + 1) % std::max(itr / 10, 1)) == 0)) {
			const auto logpath = learnlogdir + "/" + std::to_string(counter_itr);
			std::filesystem::create_directories(logpath);
			Evaluator::save(logpath);
			std::ofstream fs(logpath + "/log.txt");
			if (fs) {
				fs << rootnum << "\n" << learnedposnum << "\n";
			}
		}
		dw.save(tempgrad);
		counter_batch = 0;
	}
	std::ofstream fs("./log.txt", std::ios_base::app);
	if (fs) {
		fs << datestring << " batch:" << batch << " iterate:" << itr <<
			" roots:" << rootnum << " learnedpos:" << learnedposnum << "\n";
	}
	//保険セーブ消去
	std::filesystem::remove(tempinfo);
	std::filesystem::remove(tempgrad);
	std::cout << "learning end." << std::endl;
}

