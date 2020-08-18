#include "joseki.h"
#include "usi.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <Windows.h>

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

//定跡書き出し
void Joseki::josekiOutput(const std::vector<SearchNode*> const history)  {
	setOutputFileName("treejoseki_output");
	std::cout << timerStart() << std::endl;
	
	//書き出しファイルオープン
	FILE* fp;
	fopen_s(&fp, (outputFileName).c_str(), "wb");
	//fopen_s(&fp, "testjoseki.txt", "w");

	//書き出しノード保存用キュー
	std::queue<SearchNode*> nq;
	nq.push(history.front());
	
	//ノードの数を数え、infoファイルに出力する
	size_t nodeCount = SearchNode::sortChildren(nq.front());
	std::ofstream ofs(outputFileInfoName);
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
	if (false) {
		while (!nq.empty()) {
			const SearchNode* const node = nq.front();	//キューの先頭からノード取り出し
			nq.pop();
			for (const auto c : node->children) {	//注目ノードの子ノードをキューに収める
				nq.push(c);
			}
			josekinode jNode(index, node->getState(), node->move.getU(), node->getMass(), node->getEvaluation(), node->children.size(), childIndex);

			fwrite(&jNode, sizeof(jNode), 1, fp);

			childIndex += jNode.childCount;

			index++;
			//if (index % (nodeCount / 10) == 0) {	//途中経過
			//	std::cout << (index / (nodeCount / 10) * 10) << "%,Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
			//}
		}
	}
	else {
		SearchNode** nodes = (SearchNode**)malloc(sizeof(SearchNode*) * nodeCount);
		josekinode* jn = (josekinode*)malloc(sizeof(josekinode) * nodeCount);
		nodes[0] = history.front();
		for (index = 0; index < nodeCount; ++index) {
			const auto node = nodes[index];	//nodesから注目ノードを取り出し
			const auto childCount = node->children.size();
			jn[index] = josekinode(index, node->getState(), node->move.getU(), node->getMass(), node->getEvaluation(), childCount, childIndex);	//注目ノードをjnに収める
			for (int i = 0; i < childCount;++i) {	//子ノードをnodesに格納
				nodes[childIndex + i] = node->children[i];
			}
			childIndex += childCount;	//子ノードの数だけchildIndexを進める
			//if (index % (nodeCount / 10) == 0) {	//途中経過
			//	std::cout << (index / (nodeCount / 10) * 10) << "%,Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
			//}
		}

		fwrite(jn, sizeof(jn[0]), nodeCount, fp);	//一気に書き出し

		free(jn);
		free(nodes);
	}

	//時間出力
	ofs.open(outputFileInfoName,std::ios::app);
	ofs << "Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
	ofs.close();
	fclose(fp);
}

void Joseki::josekiInput(SearchTree* tree) {
	setInputFileName("treejoseki_input");
	std::cout << timerStart() << std::endl;

	inputfile inputFile;
	inputFile.open(inputFileName, inputFileInfoName);
	std::cout << "Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;

	std::cout << "ノード数:" << inputFile.nodeCount << std::endl;

	//すぐ下の子ノード達だけ展開する
	int depth = 0;
	std::vector<size_t>childrenToThread = yomikomiDepth(inputFile, 0, depth);

	//子ノードの数だけ再帰的に読みこむ
	std::vector<std::thread> thr;
	for (auto c : childrenToThread) {
		thr.push_back(std::thread(&Joseki::yomikomiRecursive, this, std::ref(inputFile), c));
	}
	std::cout << "スレッド数：" << thr.size() << std::endl;
	for (int i = 0; i < thr.size(); ++i) {
		thr[i].join();
	}

	//for (int i = 1; i < inputFile.nodeCount; ++i) {
	//	inputFile.nodes[inputFile.parents[i]]->children.push_back(inputFile.nodes[i]);
	//}

	josekiNodes = &(inputFile.josekiNodes[0]);
	tree->setRoot(josekiNodes,inputFile.position,inputFile.nodeCount);
	
	//josekiNodes = inputFile.nodes[0];
	////初期局面の作成(まだ初期状態から弄ってない)
	//std::vector<std::string> startpos;
	//startpos.push_back(" ");
	//startpos.push_back("startpos");
	//Kyokumen kyo = Kyokumen(startpos);
	//kyokumen = kyo;
	

	inputFile.close();

	//SearchNode::sortChildren(josekiNodes);

	std::cout << timerInterval() << "秒経過" << std::endl;
}

static std::mutex mtx;
//1行読みこむ。fpをそのまま渡す用
std::vector<size_t> Joseki::yomikomiLine(inputfile &inputFile, const size_t index) {
	std::vector<size_t> childIndexes;
	josekinode node = inputFile.josekiNodes[index];

	
	fpos_t ind = inputFile.firstPos + index * (sizeof(node));
	//読むのは\rの前まで
	//fsetpos(inputFile.fp, &ind);

	
	/*if (index != node.index) {
		std::cout << "index Error" << std::endl;
	}*/

	//size_t tIndex = node.index;
	size_t tIndex = index;

	Move move = Move(node.move);

	childIndexes.reserve(node.childCount);
	for (int i = 0; i < node.childCount; ++i) {		//子ノードのインデックスが読み終わるまでループ
		childIndexes.push_back(node.childIndex + i);
		inputFile.parents[childIndexes.back()] = tIndex;	 //親のインデックスを要素として持つ
	}
	//std::lock_guard<std::mutex>lock(mtx);
	
	inputFile.nodes[tIndex] = (SearchNode::restoreNode(move, node.st, node.eval, node.mass));
	inputFile.nodes[tIndex]->children.reserve(node.childCount);

	if (tIndex != 0) {//1つ目は親なし
		auto p = inputFile.parents[tIndex];
		auto t = inputFile.nodes[p];
		t->children.push_back(inputFile.nodes[tIndex]);
	}

	//node.viewNode();

	return childIndexes;
}

//indexとその子供を再帰的に読みこむ
void Joseki::yomikomiRecursive(inputfile &inputFile, const size_t index) {
	auto children = yomikomiLine(inputFile, index);
	for (auto c : children) {
		yomikomiRecursive(inputFile, c);
	}
}

//指定された深さまで読みこみ、子供たちを返す。0で指定されたindexのみ。並列処理はしない
std::vector<size_t> Joseki::yomikomiDepth(inputfile &inputFile, const size_t index, const int depth) {
	auto c = yomikomiLine(inputFile, index);
	if (depth == 0) {
		return c;
	}
	else {
		std::vector<size_t>children;
		for (auto ch : c) {
			auto v = yomikomiDepth(inputFile, ch, depth - 1);
			children.insert(children.end(), v.begin(), v.end());
		}
		return children;
	}
}

void Joseki::yomikomiBreath(inputfile& inputFile, const size_t index) {
	std::queue<size_t> children;
	children.push(index);
	while (!children.empty()) {
		auto t = yomikomiLine(inputFile, children.front());
		children.pop();
		for (auto c : t) {
			children.push(c);
		}
	}
}

//末尾の数字を取り除く
std::string Joseki::getSfenTrimed(std::string sfen) {
	int i = sfen.length() - 1;
	while (sfen[i] == ' ') { --i; }
	while (isdigit(sfen[i])) { --i; }
	while (sfen[i] == ' ') { --i; }
	return sfen.substr(0, i);
}

//ひとまずstdを使用して実装
void Joseki::readBook(std::string fileName) {
	std::ifstream ifs(fileName);
	while (!ifs.eof()) {
		std::string line;
		std::getline(ifs, line);
		//sfenを見つけたら格納
		if (line.length() >= 4 && line.substr(0, 4) == "sfen") {
			bookNode bn;

			//末尾の数字を取り除く
			std::string sfen = getSfenTrimed(line);
			//次の行に最善手があるので読む
			std::getline(ifs, line);
			auto column = usi::split(line, ' ');
			
			//最善手
			bn.bestMove = Move(column[0],true);
			/* 最善手以外要らない？
			//次の相手の指し手の最善手
			bn.nextMove = Move(column[1], false);
			//その指し手の価値
			bn.value = std::stod(column[2]);
			//深さ
			bn.depth = std::stod(column[3]);
			//出現回数
			bn.num = std::stod(column[4]);
			*/
			//仮に出現回数を0にしておく
			bn.num = 0;

			bookJoseki.emplace(sfen, bn);
		}
	}
}

Joseki::bookNode Joseki::getBestMove(std::string sfen)
{
	bookNode bn;	//出現回数には-1が入っている
	if (bookJoseki.find(sfen) != bookJoseki.end()) {
		return bookJoseki[sfen];
	}
	return bn;
}
