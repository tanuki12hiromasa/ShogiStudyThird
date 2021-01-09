#include "stdafx.h"
#include "tree.h"
#include "move_gen.h"
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

SearchTree::SearchTree()
	:rootPlayer(),startKyokumen()
{
	leave_branchNode = false;
	nodecount = 1;
	history.push_back(new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false)));
}

std::pair<bool, std::vector<SearchNode*>> SearchTree::set(const std::vector<std::string>& usitokens) {
	const auto moves = Move::usiToMoves(usitokens);
	return set(Kyokumen(usitokens), moves);
}
void SearchTree::makeNewTree(const std::vector<std::string>& usitokens) {
	const auto moves = Move::usiToMoves(usitokens);
	makeNewTree(Kyokumen(usitokens), moves);
}

std::pair<bool, std::vector<SearchNode*>> SearchTree::set(const Kyokumen& startpos, const std::vector<Move>& usihis) {
	std::vector<SearchNode*> newNodes;
	if (!history.empty() && (history.size() <= usihis.size()) && startKyokumen == startpos) {
		int i;
		for (i = 0; i < history.size() - 1; i++) {
			if (history[i + 1ull]->move != usihis[i]) {
				return std::make_pair(false, newNodes);
			}
		}
		for (; i < usihis.size(); i++) {
			SearchNode* root = getRoot();
			const Move nextmove = usihis[i];
			SearchNode* nextNode = nullptr;
			if (root->isLeaf()) {
				MoveGenerator::genAllMove(root, rootPlayer.kyokumen);
			}
			for (SearchNode* child : root->children) {
				if (child->move == nextmove) {
					nextNode = child;
					break;
				}
			}
			if (nextNode == nullptr) {
				nextNode = root->addChild(nextmove);
			}
			newNodes.push_back(nextNode);
			proceed(nextNode);
		}
		return std::make_pair(true, newNodes);
	}
	return std::make_pair(false, newNodes);
}

void SearchTree::makeNewTree(const Kyokumen& startpos, const std::vector<Move>& usihis) {
	history.clear();
	startKyokumen = startpos;
	history.push_back(new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false)));
	rootPlayer = SearchPlayer(startKyokumen);
	for (auto& usimove : usihis) {
		SearchNode* rootNode = getRoot();
		MoveGenerator::genAllMove(rootNode, rootPlayer.kyokumen);
		SearchNode* next = nullptr;
		for (const auto& child : rootNode->children) {
			assert(child != nullptr);
			if (child->move == usimove) {
				next = child;
				break;
			}
		}
		if (next == nullptr) {
			next = rootNode->addChild(usimove);
		}
		proceed(next);
	}
}

SearchNode* SearchTree::getBestMove()const {
	SearchNode* const rootNode = getRoot();
	return rootNode->getBestChild();
}

std::vector<SearchNode*> SearchTree::getPV()const {
	SearchNode* node = getRoot();
	std::vector<SearchNode*> pv = { node };
	while (!node->isLeaf()){
		node = node->getBestChild();
		if (node == nullptr)break;
		pv.push_back(node);
	}
	return pv;
}

void SearchTree::proceed(SearchNode* node) {
	historymap.emplace(rootPlayer.kyokumen.getHash(), std::make_pair(rootPlayer.kyokumen.getBammen(), history.size() - 1));
	rootPlayer.kyokumen.proceed(node->move);
	rootPlayer.feature.set(rootPlayer.kyokumen);
	history.push_back(node);
}

void SearchTree::deleteBranch(SearchNode* base, const std::vector<SearchNode*>& savedNodes) {
	if (leave_branchNode) return;
	for (auto saved : savedNodes) {
		for (auto node : base->children) {
			if (node != saved) {
				const size_t delnum = node->deleteTree();
				nodecount -= delnum;
			}
		}
		base = saved;
	}
}

void SearchTree::deleteTree(SearchNode* const root) {
	const size_t delnum = root->deleteTree();
	nodecount -= delnum;
	delete(root);
	nodecount--;
}
#pragma optimize("",on)

std::pair<unsigned, SearchNode*> SearchTree::findRepetition(const Kyokumen& kyokumen)const {
	auto range = historymap.equal_range(kyokumen.getHash());
	unsigned num = 0;
	size_t latest = 0;
	SearchNode* latestNode = nullptr;
	for (auto it = range.first; it != range.second; it++) {
		if (kyokumen.teban() == ((*it).second.second % 2 == 0) && (*it).second.first == kyokumen.getBammen()) {
			num++;
			if ((*it).second.second > latest) {
				latest = (*it).second.second;
				latestNode = history[latest];
			}
		}
	}
	return std::make_pair(num, latestNode);
}

void SearchTree::foutTree()const {
	time_t startTime = clock();


	std::ofstream fs("treelog.txt");
	std::queue<SearchNode*> nq;
	fs << rootPlayer.kyokumen.toSfen() << "\n";
	nq.push(history.front());
	size_t index = 0;
	size_t c_index = 1;
	while (!nq.empty()) {
		const SearchNode* const node = nq.front();
		nq.pop(); 
		int st = static_cast<int>(node->status.load());
		fs << index << "," << st << "," << node->move.toUSI() << "," << node->eval << "," << node->mass << ",[,";
		for (const auto c : node->children) {
			nq.push(c);
			fs << c_index << ",";
			c_index++;
		}
		fs << "]\n";
		index++;
	}
	fs.close();


	std::cout << "Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
}



void SearchTree::foutJoseki(int joseki_make_type,int fileCount)const {
	time_t now = time(NULL);
	struct tm pnow;
	localtime_s(&pnow, &now);
	std::cout << "開始時刻　" << pnow.tm_hour << "時" << pnow.tm_min << "分" << pnow.tm_sec << "秒" << std::endl;

	time_t startTime = clock();
	const unsigned int maxLength = 70;
	unsigned int maxLengthTemp = maxLength;
	bool loop = true;

	while (loop) {
		loop = false;
		FILE* fp;
		std::string fileName = "treejoseki_FILE.txt";
		if (joseki_make_type == 1) {
			fileName = "josekiFolder/treejoseki" + std::to_string(fileCount + 1) + ".txt";
		}
		fopen_s(&fp, fileName.c_str(), "w");
		std::queue<SearchNode*> nq;
		nq.push(history.front());
		size_t index = 0;
		size_t c_index = 1;
		int childCount;
		int lineLen = maxLengthTemp;

		

		//1加算は改行の分
		fprintf_s(fp, "lineLen,%lu,nodeCount,%lu\n", lineLen + 1, SearchNode::sortChildren(nq.front()));

		fprintf_s(fp, "%s\n", startKyokumen.toSfen().c_str());


		char sp[1024];
		while (!nq.empty()) {
			const SearchNode* const node = nq.front();
			nq.pop();
			const int st = static_cast<int>(node->status.load());

			childCount = 0;

			for (const auto c : node->children) {
				nq.push(c);
				childCount++;
			}
			unsigned int stringLength = sprintf_s(sp, "%lu,%d,%d,%lf,%lf,%d,%lu\n", index, st, node->move.getU(), node->eval.load(), node->mass.load(), childCount, c_index);
			fprintf_s(fp, "%*s", lineLen, sp);

			if (maxLengthTemp < stringLength) {
				maxLengthTemp = stringLength;
				loop = true;
			}

			c_index += childCount;

			index++;
			if (index % (getNodeCount() / 10) == 0) {
				std::cout << (index / (getNodeCount() / 10) * 10) << "%,Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
			}
		}
		if (loop) {
			std::cout << "想定よりも長い行があったため再度出力します。" << std::endl;
		}
		fclose(fp);
	}
}
void SearchTree::foutJosekiBin(int joseki_make_type, int fileCount)const {
	time_t now = time(NULL);
	struct tm pnow;
	localtime_s(&pnow, &now);
	std::cout << "開始時刻　" << pnow.tm_hour << "時" << pnow.tm_min << "分" << pnow.tm_sec << "秒" << std::endl;

	time_t startTime = clock();
	size_t index = 0;
	int st;
	uint16_t move;
	double mass;
	double eval;
	int childCount;
	size_t c_index = 1;
	const unsigned int maxLength = sizeof(index) + sizeof(st) + sizeof(move) + sizeof(eval) + sizeof(mass) + sizeof(childCount) + sizeof(c_index);
	unsigned int maxLengthTemp = maxLength;

	FILE* fp;
	std::string fileName = "treejoseki_FILE";
	if (joseki_make_type == 1) {
		fileName = "josekiFolder/treejoseki" + std::to_string(fileCount + 1);
	}
	fopen_s(&fp, (fileName + ".bin").c_str(), "wb");
	std::queue<SearchNode*> nq;
	nq.push(history.front());
	int lineLen = maxLengthTemp;


	fprintf_s(fp, "lineLen,%lu,nodeCount,%lu\n", lineLen, SearchNode::sortChildren(nq.front()));
	fprintf_s(fp, "%s\n", startKyokumen.toSfen().c_str());

	char* sp = (char*)malloc(maxLength + 1);	//ゼロ文字の分1増やしている
	while (!nq.empty()) {
		const SearchNode* const node = nq.front();
		nq.pop();
		index;
		st = static_cast<int>(node->status.load());
		move = node->move.getU();
		eval = node->eval.load();
		mass = node->mass.load();
		childCount;
		c_index;

		childCount = 0;

		for (const auto c : node->children) {
			nq.push(c);
			childCount++;
		}
		//sprintf_s(sp, "%lu,%d,%d,%lf,%lf,%d,%lu\n", index, st, node->move.getU(), node->eval.load(), node->mass.load(), childCount, c_index);
		fwrite(&index, sizeof(index), 1, fp);
		fwrite(&st, sizeof(st), 1, fp);
		fwrite(&move, sizeof(move), 1, fp);
		fwrite(&eval, sizeof(eval), 1, fp);
		fwrite(&mass, sizeof(mass), 1, fp);
		fwrite(&childCount, sizeof(childCount), 1, fp);
		fwrite(&c_index, sizeof(c_index), 1, fp);

		//fprintf_s(fp, "%*s", lineLen, sp);
		//fprintf_s(fp, "%s", sp);


		c_index += childCount;

		index++;
		if (index % (getNodeCount() / 10) == 0) {
			std::cout << (index / (getNodeCount() / 10) * 10) << "%,Time:" << (clock() - startTime) / (double)CLOCKS_PER_SEC << "秒経過" << std::endl;
		}
	}
	fclose(fp);

}
void SearchTree::foutJoseki()const {
	foutJoseki(0, 0);
}

void SearchTree::setRoot(SearchNode* const root, std::vector<std::string> tokens ,size_t nodes) {
	//makeNewTree(tokens);
	//history.push_back(root);

	history.clear();
	startKyokumen = Kyokumen(tokens);
	history.push_back(root);
	rootPlayer = SearchPlayer(startKyokumen);
	const auto usihis = Move::usiToMoves(tokens);
	for (auto& usimove : usihis) {
		SearchNode* rootNode = getRoot();
		//MoveGenerator::genAllMove(rootNode, rootPlayer.kyokumen);
		SearchNode* next = nullptr;
		for (const auto& child : rootNode->children) {
			assert(child != nullptr);
			if (child->move == usimove) {
				next = child;
				break;
			}
		}
		if (next == nullptr) {
			next = rootNode->addChild(usimove);
		}
		proceed(next);
	}
	nodecount = nodes;
}

void SearchTree::setRoot(SearchNode* const root, const Kyokumen& kyokumen, size_t nodes) {
	SearchNode* motoroot = history.front();
	motoroot->deleteTree();
	delete motoroot;
	history.clear();
	history.push_back(root);				//std::cout << root->eval << std::endl;
	startKyokumen = kyokumen;				//std::cout << "sfen " << kyokumen.toSfen() << std::endl;
	rootPlayer = SearchPlayer(kyokumen);    //ここ 解決したはず
	nodecount = nodes;						//std::cout << "nodes "<< nodes << std::endl;
}
