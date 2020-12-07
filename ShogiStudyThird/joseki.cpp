﻿#include "joseki.h"
#include "usi.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <Windows.h>
#include <climits>
#include <random>

//定跡フォルダーの中のファイル数を数える
static int getFileCount() {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	auto target = L"josekiFolder/*.bin";
	int fileCount = 0;
	hFind = FindFirstFile(target, &findFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			++fileCount;
			//_tprintf(TEXT("%d: %s\n"), ++fileCount, findFileData.cFileName);
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}
	return fileCount;
}

void Joseki::setOption(std::vector<std::string> tokens){
	auto t = tokens[2];
	if (t == "joseki_on") {
		yomikomi_on = (tokens[4] == "true");
	}
	else if (t == "josekifoldername") {
		josekiFolderName = tokens[4];
		loopFileName = josekiFolderName + "\\next.txt";
	}
	else if (t == "josekiinputname") {
		setInputFileName(tokens[4]);
	}
	else if (t == "josekiinputfilename") {
		inputFileName = josekiFolderName + "\\" + tokens[4];
	}
	else if (t == "josekiinputinfofilename") {
		inputFileInfoName = josekiFolderName + "\\" + tokens[4];
	}
	else if (t == "josekioutputname") {
		setOutputFileName(tokens[4]);
	}
	else if (t == "josekikakidashi") {
		kakidashi_on = (tokens[4] == "true");
	}
	else if (t == "joseki_loop") {
		joseki_loop = (tokens[4] == "true");
	}
	else if (t == "joseki_loop_interval") {
		joseki_loop_interval = std::stoi(tokens[4]);
	}
	else if (t == "pruningborder") {
		if (tokens[4][0] == 'e') {
			pruningBorder = pow(10.0, std::stod(tokens[4].substr(2,tokens[4].size() - 1)));
		}
		else {
			pruningBorder = std::stod(tokens[4]);
		}
	}
	else if (t == "pruningborderEval") {
		pruningBorderEval = std::stod(tokens[4]);
	}
	else if (t == "pruning_on") {
		pruning_on = (tokens[4] == "true");
	}
	else if (t == "pruning_type") {
		pruning_type = std::stoi(tokens[4]);
	}
	else if (t == "joseki_backup_T_e") {
		backup_T_e = std::stoi(tokens[4]);
	}
	else if (t == "joseki_backup_T_d") {
		backup_T_d = std::stoi(tokens[4]);
	}
	else if (t == "pruning_depth") {
		pruning_depth = std::stoi(tokens[4]);
	}
	else if (t == "pruning_T_c") {
		pruning_T_c = std::stoi(tokens[4]);
	}
	else if (t == "leaveNodeCount") {
		leaveNodeCount = std::stoi(tokens[4]);
	}
}
void Joseki::printOption() {
	std::cout << "option name joseki_on type check default false" << std::endl;
	std::cout << "option name joseki_loop type check default false" << std::endl;
	std::cout << "option name joseki_loop_interval type string default 0" << std::endl;
	std::cout << "option name josekifoldername type string default joseki" << std::endl;
	std::cout << "option name josekiinputname type string default foutjoseki" << std::endl;
	std::cout << "option name josekioutputname type string default foutjoseki" << std::endl;
	std::cout << "option name josekikakidashi type check default false" << std::endl;
	std::cout << "option name pruningborder type string default 100" << std::endl;
	std::cout << "option name pruningborderEval type string default 100" << std::endl;
	std::cout << "option name pruning_on type check default false" << std::endl;
	std::cout << "option name pruning_type type string default 0" << std::endl;
	std::cout << "option name joseki_backup_T_e type string default 1" << std::endl;
	std::cout << "option name joseki_backup_T_d type string default 1" << std::endl;
	std::cout << "option name pruning_depth type string default 5" << std::endl;
	std::cout << "option name pruning_T_c type string default 40" << std::endl;
	std::cout << "option name leaveNodeCount string default 0" << std::endl;
}

void Joseki::josekiOutputIGameOver(const std::vector<SearchNode*> const history,std::vector<std::string> tokens) {
	if (kakidashi_on) {
		result = tokens[1];
		if (endBattleResult != 0) {
			result = (endBattleResult == 1) ? "win" : "lose";
		}

		backUp(history);
		josekiOutput(history);
		//std::cout << "josekiinfoname " << outputFileInfoName << std::endl;
		//std::cout << "josekiname " << outputFileName << std::endl;
		
	}
}

//定跡書き出し
void Joseki::josekiOutput(const std::vector<SearchNode*> const history)  {
	std::cout << timerStart() << std::endl;
	
	std::string moveHis = "";
	std::string usiHis = "";
	for (SearchNode* his : history) {
		moveHis += std::to_string(his->move.getU());
		moveHis += ",";
		if (his->move.toUSI() == "nullmove" || his->move.toUSI() == "1a1a") {
			usiHis += "position startpos moves";
		}
		else {
			usiHis += his->move.toUSI();
		}
		usiHis += " ";
	}

	size_t pruningedNodeCount = 0;
	if (pruning_on) {
		pruningedNodeCount = pruning(history[0]);
		std::cout << "枝刈りされたノードの数：" << pruningedNodeCount << std::endl;
	}

	//書き出しノード保存用キュー
	std::queue<SearchNode*> nq;
	nq.push(history.front());
	
	//ノードの数を数え、infoファイルに出力する
	//std::cout << "ソート完了" << std::endl;
	size_t nodeCount = SearchNode::sortChildren(nq.front());
	


	std::ofstream ofs(outputFileInfoName);
	ofs << "nodeCount," << nodeCount << std::endl;
	ofs << moveHis << std::endl;
	ofs << usiHis << std::endl;
	ofs << "depth," << nq.front()->getMass() << std::endl;

	//ノードの数が多すぎるとメモリの限界を超えるため、出力を中止する
	ofs << "nodesize" << "," << sizeof(josekinode) << std::endl;
	size_t fileSize = nodeCount * sizeof(josekinode);
	size_t gigabyte = 1024 * 1024 * 1024;
	size_t maxByte = 10 * gigabyte;
	ofs << "推定ファイルサイズ：" << std::to_string(fileSize) << "バイト(" << (double)fileSize / gigabyte << "ギガバイト)" << std::endl;
	std::cout << "推定ファイルサイズ：" << std::to_string(fileSize) << "バイト" << std::endl;
	std::cout << "推定ファイルサイズ：" << std::to_string((double)fileSize / gigabyte) << "ギガバイト" << std::endl;
	if (fileSize >= maxByte) {
		std::cout << "書き出そうとしているファイルサイズが大きすぎます。" << std::endl;
		std::cout << "最大サイズ：" << maxByte << std::endl;
		std::cout << "出力を中止します。" << std::endl;
		ofs.close();
		nextSubForJosekiLoop();
		return;
	}

	ofs << "result " << result << std::endl;

	//枝刈りに関する情報
	ofs << "枝刈りの有無：" << (pruning_on ? "有り" : "無し") << std::endl;
	if (pruning_on) {
		ofs << "枝刈りタイプ：" << pruning_type << std::endl;
		ofs << "枝刈りボーダー：" << pruningBorder << std::endl;
		ofs << "枝刈りされたノードの数：" << pruningedNodeCount << std::endl;
		ofs << "削減されたバイト数：" << pruningedNodeCount * sizeof(josekinode) << "(" << (double)(pruningedNodeCount * sizeof(josekinode)) / gigabyte << "GB)" << std::endl;
	}

	ofs.close();	//書き出し中にファイル情報を確認するため、いったん閉じる

	//ここからファイルに書き出し
	size_t index = 0;
	size_t childIndex = 1;

	SearchNode** nodes = (SearchNode**)malloc(sizeof(SearchNode*) * nodeCount);
	josekinode* jn = (josekinode*)malloc(sizeof(josekinode) * nodeCount);
	nodes[0] = history.front();
	for (index = 0; index < nodeCount; ++index) {
		const SearchNode * node = nodes[index];	//nodesから注目ノードを取り出し
		const size_t childCount = node->children.size();
		SearchNode::State state = node->getState();
		if (childCount > 0) {
			state = SearchNode::State::Expanded;
		}
		jn[index] = josekinode(index, state, node->move.getU(), node->getMass(), node->getEvaluation(), childCount, childIndex);	//注目ノードをjnに収める
		for (int i = 0; i < childCount;++i) {	//子ノードをnodesに格納
			nodes[childIndex + i] = node->children[i];
		}
		childIndex += childCount;	//子ノードの数だけchildIndexを進める
		if (index % (nodeCount / 10) == 0) {	//途中経過
			std::cout << (index / (nodeCount / 10) * 10) << "%,Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
		}
	}

	//書き出しファイルオープン
	FILE* fp;
	fopen_s(&fp, (outputFileName).c_str(), "wb");
	fwrite(jn, sizeof(jn[0]), nodeCount, fp);	//一気に書き出し
	fflush(fp);
	free(jn);
	/*for (int i = 0; i < nodeCount; ++i) {
		free(nodes[i]);
	}*/
	free(nodes);

	//時間出力
	ofs.open(outputFileInfoName,std::ios::app);
	ofs << "Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒で出力完了" << std::endl;
	ofs.close();
	fclose(fp);

	if (joseki_loop) {
		nextForJosekiLoop(1);
	}
}

void Joseki::backUp(std::vector<SearchNode*> history)
{
	std::cout << "バックアップを行います。" << std::endl;
	const double MateScoreBound = 30000.0;
	typedef std::pair<double, double> dd;
	double T_e = backup_T_e;
	double T_d = backup_T_d;
	SearchNode* node = history.back();

	for (int i = history.size() - 2; i >= 0; i--) {
		node = history[i];
		double emin = 99999;
		std::vector<dd> emvec;
		for (const auto& child : node->children) {
			const double eval = child->getEvaluation();
			const double mass = child->mass;
			emvec.push_back(std::make_pair(eval, mass));
			if (eval < emin) {
				emin = eval;
			}
		}
		if (std::abs(emin) > MateScoreBound) {
			node->setMateVariation(emin);
		}
		else {
			double Z_e = 0;
			double Z_d = 0;
			for (const auto& em : emvec) {
				const double eval = em.first;
				Z_e += std::exp(-(eval - emin) / T_e);
				Z_d += std::exp(-(eval - emin) / T_d);
			}
			double E = 0;
			double M = 1;
			for (const auto& em : emvec) {
				const double eval = em.first;
				const double mass = em.second;
				E -= eval * std::exp(-(eval - emin) / T_e) / Z_e;
				M += mass * std::exp(-(eval - emin) / T_d) / Z_d;
			}
		
			//std::cout << "[" << i << "](" << node->eval << "→";
			//node->setEvaluation(E * (i == history.size() - 2 ? (double)i * i : 1));
			node->setEvaluation(E);
			node->setMass(M);
			//std::cout << node->eval << ") ";
		}
	}
	std::cout << std::endl;
	std::cout << "バックアップ完了" << std::endl;
}

//定跡書き出し
void Joseki::josekiTextOutput(const std::vector<SearchNode*> const history) {
	std::cout << timerStart() << std::endl;


	//書き出しファイルオープン
	FILE* fp;
	fopen_s(&fp, "joseki\\joseki_text_output.txt", "w");

	//書き出しノード保存用キュー
	std::queue<SearchNode*> nq;
	nq.push(history.front());

	//ノードの数を数え、infoファイルに出力する
	size_t nodeCount = SearchNode::sortChildren(nq.front());
	std::ofstream ofs("joseki\\joseki_text_output_info.txt");
	ofs << "nodeCount," << nodeCount << std::endl;
	for (auto his : history) {
		ofs << his->move.getU();
		ofs << ",";
	}
	ofs << std::endl;
	ofs << "position ";
	
	for (auto his : history) {
		if (his->move.toUSI() == "nullmove") {
			ofs << "startpos moves";
		}
		else {
			ofs << his->move.toUSI();
		}
		ofs << " ";
	}
	ofs << std::endl;

	ofs << "depth," << nq.front()->getMass() << std::endl;

	ofs.close();	//書き出し中にファイル情報を確認するため、いったん閉じる

	//ここからファイルに書き出し
	size_t index = 0;
	size_t childIndex = 1;

	SearchNode** nodes = (SearchNode**)malloc(sizeof(SearchNode*) * nodeCount);
	josekinode* jn = (josekinode*)malloc(sizeof(josekinode) * nodeCount);
	nodes[0] = history.front();
	fprintf(fp, "インデックス,展開状態,指し手,USI指し手,探索深さ指標,価値,子ノードの数,子ノードの先頭インデックス\n");
	for (index = 0; index < nodeCount; ++index) {
		const auto node = nodes[index];	//nodesから注目ノードを取り出し
		const auto childCount = node->children.size();
		SearchNode::State state = node->getState();
		if (childCount > 0) {
			state = SearchNode::State::Expanded;
		}
		jn[index] = josekinode(index, state, node->move.getU(), node->getMass(), node->getEvaluation(), childCount, childIndex);	//注目ノードをjnに収める
		for (int i = 0; i < childCount; ++i) {	//子ノードをnodesに格納
			nodes[childIndex + i] = node->children[i];
		}
		fprintf(fp,"%d,%d,%d,%s,%f,%f,%d,%d\n", index, state, node->move.getU(),node->move.toUSI().c_str(), node->getMass(), node->getEvaluation(), childCount, childIndex);
		childIndex += childCount;	//子ノードの数だけchildIndexを進める
	}

	//fwrite(jn, sizeof(jn[0]), nodeCount, fp);	//一気に書き出し

	free(jn);
	free(nodes);

	//時間出力
	ofs.open(outputFileInfoName, std::ios::app);
	ofs << "Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
	ofs.close();
	fclose(fp);
}


void Joseki::josekiInput(SearchTree* tree) {
	if (yomikomi_on == false) {
		return;
	}
	if (joseki_loop) {
		nextForJosekiLoop(-1);
	}

	//定跡の情報が入ったファイルを開く
	std::ifstream ifs(inputFileInfoName);
	if (!ifs.is_open()) {
		std::cout << inputFileInfoName << "が開けませんでした。" << std::endl;
		std::cout << "定跡なしで開始します。" << std::endl;
		return;
		//exit(EXIT_FAILURE);
	}

	//ノード数の取得
	std::string nodeCountStr;
	std::getline(ifs, nodeCountStr);
	nodeCount = std::stol(usi::split(nodeCountStr, ',')[1]);    //infoファイルからノード数を取得

	//positionの取得
	std::string pos;
	std::getline(ifs, pos); //moveが保存された行
	std::getline(ifs, pos); //positionが保存された行
	//tokenOfPosition = usi::split(pos, ' ');
	tokenOfPosition.push_back("position");
	tokenOfPosition.push_back("startpos");
	tokenOfPosition.push_back("moves");

	ifs.close();

	//定跡本体を開く
	errno_t err = fopen_s(&fp, (inputFileName).c_str(), "rb");
	if (err) {
		std::cout << inputFileName << "が開けませんでした。" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	std::cout << timerStart() << std::endl;

	std::cout << "Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;

	std::cout << "ノード数:" << nodeCount << std::endl;

	nodesForProgram = (SearchNode**)calloc(nodeCount, sizeof(SearchNode*));	//プログラム内で使用するnode
	parentsIndex = (size_t*)calloc(nodeCount, sizeof(size_t));
	nodesFromFile = (josekinode*)calloc(nodeCount, sizeof(josekinode));	//定跡の復元に一時的に利用するノード
	fread(nodesFromFile, sizeof(josekinode), nodeCount, fp);	//定跡本体をバイナリファイルから読み込み

	//すぐ下の子ノード達だけ展開する
	int depth = 0;
	std::vector<size_t>childrenToThread = yomikomiDepth(0, depth);
	if (childrenToThread.size() <= 0) {
		std::cout << "子ノードがありません。" << std::endl;
		std::cout << "プログラムを強制終了します。" << std::endl;
		exit(EXIT_FAILURE);
	}

	//子ノードの数だけ再帰的に読みこむ
	std::vector<std::thread> thr;
	for (auto c : childrenToThread) {
		thr.push_back(std::thread(&Joseki::yomikomiRecursive, this, c));
	}
	std::cout << "スレッド数：" << thr.size() << std::endl;
	for (int i = 0; i < thr.size(); ++i) {
		thr[i].join();
	}

	root = nodesForProgram[0];
	nodeCount = SearchNode::sortChildren(root);

	tree->setRoot(root,tokenOfPosition,nodeCount);
	
	free(nodesFromFile);
	free(parentsIndex);
	free(nodesForProgram);
	fclose(fp);

	std::cout << timerInterval() << "秒経過" << std::endl;
}

static std::mutex mtx;
//1行読みこむ。fpをそのまま渡す用
std::vector<size_t> Joseki::yomikomiLine(const size_t index) {
	std::vector<size_t> childIndexes;
	josekinode node = nodesFromFile[index];

	
	size_t tIndex = index;

	Move move = Move(node.move);

	childIndexes.reserve(node.childCount);
	for (int i = 0; i < node.childCount; ++i) {		//子ノードのインデックスが読み終わるまでループ
		childIndexes.push_back(node.childIndex + i);
		parentsIndex[childIndexes.back()] = tIndex;	 //親のインデックスを要素として持つ
	}
	
	nodesForProgram[tIndex] = (SearchNode::restoreNode(move, node.st, node.eval, node.mass));
	nodesForProgram[tIndex]->children.reserve(node.childCount);

	if (tIndex != 0) {//1つ目は親なし
		auto p = parentsIndex[tIndex];
		auto t = nodesForProgram[p];
		t->children.push_back(nodesForProgram[tIndex]);
	}

	return childIndexes;
}

//indexとその子供を再帰的に読みこむ
void Joseki::yomikomiRecursive(const size_t index) {
	auto children = yomikomiLine(index);
	for (auto c : children) {
		yomikomiRecursive(c);
	}
}

//指定された深さまで読みこみ、子供たちを返す。0で指定されたindexのみ。並列処理はしない
std::vector<size_t> Joseki::yomikomiDepth(const size_t index, const int depth) {
	auto c = yomikomiLine(index);
	if (depth == 0) {
		return c;
	}
	else {
		std::vector<size_t>children;
		for (auto ch : c) {
			auto v = yomikomiDepth(ch, depth - 1);
			children.insert(children.end(), v.begin(), v.end());
		}
		return children;
	}
}

void Joseki::yomikomiBreath(const size_t index) {
	std::queue<size_t> children;
	children.push(index);
	while (!children.empty()) {
		auto t = yomikomiLine(children.front());
		children.pop();
		for (auto c : t) {
			children.push(c);
		}
	}
}

size_t Joseki::pruning(SearchNode* root){
	size_t r = 0;

	//pruning_typeが4なら、探索木が大きくなければ枝刈りしない
	if (pruning_type == 4) {
		
		//ノードの数が多すぎるとメモリの限界を超えるため、出力を中止する
		size_t fileSize = treeNodeCount * sizeof(josekinode);
		size_t gigabyte = 1024 * 1024 * 1024;
		size_t maxByte = 10 * gigabyte;

		//上限まで行ってないので枝刈りせず終了。
		if (fileSize < maxByte) {
			std::cout << "ファイルサイズが小さいので枝刈りはしません" << std::endl;
			return r;
		}
	}

	std::cout << "枝刈りを行います(ノード数:" << treeNodeCount << ")" << std::endl;

	std::vector<SearchNode*>history;
	r = partialPruning(root,history,1,0);
	
	return r;
}

size_t Joseki::partialPruning(SearchNode* node, std::vector<SearchNode*> history, double select,int depth,double backupRate){
	size_t r = 0;
	//末端ならその場で終了
	if (node->children.size() == 0) {
		return r;
	}
	double mass = node->getMass();
	history.push_back(node);
	//枝刈り判定を行う
	if (isPruning(node,select,depth,backupRate)) {
		r += pruningExecuter(node, history);
	}
	else {
		if (pruning_type == 0 || pruning_type == 4 || pruning_type == 6) {
			//実現確率の計算
			if (select != -1) {
				double CE = node->children[0]->getEvaluation();
				//評価値の最大値の取得
				for (const SearchNode* child : node->children) {
					if (CE < child->getEvaluation()) {
						CE = child->getEvaluation();
					}
				}
				//バックアップ温度
				double T_c = pruning_T_c;
				double ntc = T_c - depth * 2.0;
				if (pruning_type == 6) {
					if (ntc < 2) {
						ntc = 2;
					}
					T_c = ntc;
				}
				double Z = 0;
				for (const SearchNode* child : node->children) {
					Z += std::exp(-(child->getEvaluation() - CE) / T_c);
				}
				for (int i = 0; i < node->children.size();++i) {
					//再帰的に枝刈りを行う
					SearchNode* child = node->children[i];
					double s;
					if (leaveNodeCount == 0 || i < leaveNodeCount) {
						s = std::exp(-(child->getEvaluation() - CE) / T_c) / Z;
					}
					else {
						s = 0;
					}
					r += partialPruning(child, history, s * select, depth + 1);
				}
			}
			else {
				//再帰的に枝刈りを行う
				for (SearchNode* child : node->children) {
					r += partialPruning(child, history);
				}
			}
		}
		else if (pruning_type == 2 || pruning_type == 3) {
			//実現確率の計算
			if (select != -1) {
				double CE = node->children[0]->getEvaluation();
				//評価値の最大値の取得
				for (const SearchNode* child : node->children) {
					if (CE < child->getEvaluation()) {
						CE = child->getEvaluation();
					}
				}
				//バックアップ温度
				double T_c = pruning_T_c;
				double Z = 0;
				for (const SearchNode* child : node->children) {
					Z += std::exp(-(child->getEvaluation() - CE) / T_c);
				}
				for (SearchNode* child : node->children) {
					//再帰的に枝刈りを行う
					double s = std::exp(-(child->getEvaluation() - CE) / T_c) / Z;
					double sr = select * (depth < pruning_depth ? 1 : s);
					r += partialPruning(child, history, sr, depth + 1,s);
				}
			}
		}
		else if (pruning_type == 5) {
			//再帰的に枝刈りを行う
			for (SearchNode* child : node->children) {
				r += partialPruning(child, history);
			}
		}
	}
	return r;
}

size_t Joseki::pruningExecuter(SearchNode* node, std::vector<SearchNode*> history){
	
	size_t r = node->deleteTree();
	node->setState(SearchNode::State::NotExpanded);

	return r;
}

bool Joseki::isPruning(SearchNode* node,double select,int depth,double backupRate){
	//実現確率がpruningBorder%以下なら切り捨て
	switch (pruning_type)
	{
	case 0:
	case 4:
	case 6:
	default:
		if (select < pruningBorder * 0.01) {
			return true;
		}
		break;
	case 1:
		if (node->getMass() < pruningBorder) {
			return true;
		}
		break;
	case 2:
		if (depth >= pruning_depth && select < pruningBorder * 0.01) {
			return true;
		}
		break;
	case 3:
		if ((depth >= pruning_depth && select < pruningBorder * 0.01) || (depth < pruning_depth && backupRate < pruningBorderEval * 0.01)) {
			return true;
		}
		break;
	case 5:
		if (abs(node->getEvaluation()) > pruningBorder) {
			return true;
		}
		break;
	}
	return false;
}
