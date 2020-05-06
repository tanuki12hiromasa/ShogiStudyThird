#include "joseki.h"
#include "usi.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <Windows.h>

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

void Joseki::josekiOutput(SearchNode* root,const Kyokumen rootKyokumen)  {
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
	auto fc = fileCount;
	setOutputFileName(("josekiFolder/treejoseki_" + std::to_string(fc)));

	std::cout << timerStart() << std::endl;
	
	FILE* fp;
	fopen_s(&fp, (outputFileName).c_str(), "wb");
	std::queue<SearchNode*> nq;
	nq.push(root);

	size_t nodeCount = SearchNode::sortChildren(nq.front());
	std::ofstream ofs(outputFileInfoName);
	ofs << "nodeCount," << nodeCount << std::endl;
	ofs << rootKyokumen.toSfen() << std::endl;

	size_t index = 0;
	size_t childIndex = 1;
	while (!nq.empty()) {
		const SearchNode* const node = nq.front();
		nq.pop();
		for (const auto c : node->children) {
			nq.push(c);
		}
		josekinode jNode(index, node->getState(), node->move.getU(), node->getMass(), node->getEvaluation(), node->children.size(), childIndex);

		fwrite(&jNode, sizeof(jNode), 1, fp);

		childIndex += jNode.childCount;

		index++;
		if (index % (nodeCount / 10) == 0) {
			std::cout << (index / (nodeCount / 10) * 10) << "%,Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
		}
	}
	ofs << "Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
	ofs.close();
	fclose(fp);


}

void Joseki::josekiInput() {
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
	auto fc = fileCount - 1;
	setInputFileName(("josekiFolder/treejoseki_" + std::to_string(fc)));

	std::cout << timerStart() << std::endl;

	inputfile inputFile;
	inputFile.open(inputFileName, inputFileInfoName);
	std::cout << "Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;

	if (true) {
		int depth = 0;
		std::vector<size_t>childrenToThread = yomikomiDepth(inputFile, 0, depth);


		//再帰的に読みこむ
		std::vector<std::thread> thr;
		for (auto c : childrenToThread) {
			thr.push_back(std::thread(&Joseki::yomikomiRecursive, this, std::ref(inputFile), c));
		}
		std::cout << "スレッド数：" << thr.size() << std::endl;
		for (int i = 0; i < thr.size(); ++i) {
			thr[i].join();
		}
	}
	else {
		yomikomiRecursive(inputFile, 0);
	}

	//for (int i = 1; i < inputFile.nodeCount; ++i) {
	//	inputFile.nodes[inputFile.parents[i]]->children.push_back(inputFile.nodes[i]);
	//}

	josekiNodes = inputFile.nodes[0];
	//初期局面の作成(まだ初期状態から弄ってない)
	std::vector<std::string> startpos;
	startpos.push_back(" ");
	startpos.push_back("startpos");
	Kyokumen kyo = Kyokumen(startpos);
	kyokumen = kyo;
	

	inputFile.close();

	SearchNode::sortChildren(josekiNodes);

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

	
	if (index != node.index) {
		std::cout << "index Error" << std::endl;
	}

	size_t tIndex = node.index;

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
