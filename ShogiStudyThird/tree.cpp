#include "stdafx.h"
#include "tree.h"
#include "move_gen.h"
#include <queue>
#include <fstream>

SearchTree::SearchTree()
	:rootPlayer(),startKyokumen()
{
	leave_branchNode = false;
	history.push_back(new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false)));
	nodecount = 1;
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
				const auto moves = MoveGenerator::genAllMove(root->move, rootPlayer.kyokumen);
				root->addChildren(moves);
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
		const auto moves = MoveGenerator::genAllMove(rootNode->move, rootPlayer.kyokumen);
		rootNode->addChildren(moves);
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
		fs << index << ", " << st << ", " << node->move.toUSI() << ", " << node->eval << ", " << node->mass << ", [,";
		for (const auto c : node->children) {
			nq.push(c);
			fs << c_index << ",";
			c_index++;
		}
		fs << "]\n";
		index++;
	}
	fs.close();
}