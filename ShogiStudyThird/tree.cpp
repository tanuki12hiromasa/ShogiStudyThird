#include "stdafx.h"
#include "tree.h"
#include "move_gen.h"
#include <queue>
#include <fstream>

SearchTree::SearchTree()
	:rootPlayer(), startKyokumen(), 
	leave_branchNode(false),continuous_tree(true)
{
}

SearchTree::~SearchTree() {
	while (deleteGarbage());
	auto root = getGameRoot();
	delete root;
}

void SearchTree::set(const std::vector<std::string>& usitokens) {
	const auto moves = Move::usiToMoves(usitokens);
	set(Kyokumen(usitokens), moves);
}



void SearchTree::set(const Kyokumen& startpos, const std::vector<Move>& usihis) {
	
	if (history.empty() || startKyokumen != startpos || history.size() > usihis.size() || !continuous_tree) {
		//初期状態ならmakenewtreeで初期化
		//あるいは初期局面が異なるか与えられた棋譜が内部の棋譜より短いので探索木を作り直す
		makeNewTree(startpos, usihis);
		return;
	}
	else {
		int i;
		for (i = 0; i < history.size() - 1; i++) {
			if (history[i + 1ull]->move != usihis[i]) {
				//与えられた棋譜が内部の棋譜と一致しないので探索木を作り直す
				makeNewTree(startpos, usihis);
				return;
			}
		}
		for (; i < usihis.size(); i++) {
			SearchNode* parent = getRoot();
			const Move nextmove = usihis[i];
			SearchNode* nextNode = nullptr;
			if (parent->isLeaf() || parent->isTerminal()) {
				parent->status = SearchNode::State::Expanded;
			}
			for (auto& child : parent->children) {
				//子ノードの中から棋譜での次の手を探す
				if (child.move == nextmove) {
					nextNode = &child;
					break;
				}
			}
			if (nextNode == nullptr) {
				nextNode = addNewChild(parent, nextmove);
			}
			proceed(nextNode);
		}
		return;
	}
}
void SearchTree::makeNewTree(const std::vector<std::string>& usitokens) {
	const auto moves = Move::usiToMoves(usitokens);
	makeNewTree(Kyokumen(usitokens), moves);
}

SearchNode* SearchTree::addNewChild(SearchNode* const parent, const Move& move) {
	//childrenは後から数を増やせないので作り直す
	auto p_oldchildren = parent->purge();
	auto& oldchildren = *p_oldchildren;
	std::vector<Move> moves;
	for (const auto& child : oldchildren) {
		moves.push_back(child.move);
	}
	moves.push_back(move);
	parent->addChildren(moves);
	//各子ノードの情報を入れ替える
	for (int i = 0; i < oldchildren.size(); i++) {
		auto& oldchild = oldchildren[i];
		auto& newchild = parent->children[i];
		assert(oldchild.move == newchild.move);
		newchild.swap(oldchild);
	}
	//古いchildrenは破棄する
	delete p_oldchildren;
	//追加した子ノードのポインタを返す
	return &(parent->children[parent->children.size() - 1]);
}

void SearchTree::makeNewTree(const Kyokumen& startpos, const std::vector<Move>& usihis) {
	if (!history.empty()) {
		auto root = history.front();
		if (root) {
			addGarbage(root, true);
		}
		history.clear();
	}
	startKyokumen = startpos;
	history.push_back(new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false)));
	rootPlayer = SearchPlayer(startKyokumen);
	for (const auto& usimove : usihis) {
		SearchNode* rootNode = getRoot();
		const auto moves = MoveGenerator::genAllMove(rootNode->move, rootPlayer.kyokumen);
		rootNode->addChildren(moves);
		SearchNode* next = nullptr;
		for (auto& child : rootNode->children) {
			if (child.move == usimove) {
				next = &child;
				break;
			}
		}
		if (next == nullptr) {
			next = addNewChild(rootNode, usimove);
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
	if (!leave_branchNode) {
		const auto parent = getRoot();
		for (auto& child : parent->children) {
			if (&child != node) {
				addGarbage(&child, false);
			}
		}
	}
	historymap.emplace(rootPlayer.kyokumen.getHash(), std::make_pair(rootPlayer.kyokumen.getBammen(), history.size() - 1));
	rootPlayer.kyokumen.proceed(node->move);
	rootPlayer.feature.set(rootPlayer.kyokumen);
	history.push_back(node);
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
		for (auto& c : node->children) {
			nq.push(&c);
			fs << c_index << ",";
			c_index++;
		}
		fs << "]\n";
		index++;
	}
	fs.close();
}

void SearchTree::addGarbage(SearchNode* const parent, bool deleteParent) {
	if (!parent) return;
	std::lock_guard<std::mutex> lock(mtx_deleteTrees);
	garbage_parent.push(std::make_pair(parent, deleteParent));
}

bool SearchTree::deleteGarbage() {
	SearchNode::Children* root = nullptr;
	{
		std::lock_guard<std::mutex> lock(mtx_deleteTrees);
		if (garbage_parent.empty()) {
			return false;
		}
		const auto p = garbage_parent.front(); garbage_parent.pop();
		root = p.first->purge();
		if (p.second) delete p.first;
	}
	//std::cout << "dg " << SearchNode::nodecount << " " << root->size();
	if (root) delete root;
	//std::cout << " " << SearchNode::nodecount << "\n";
	return true;
}