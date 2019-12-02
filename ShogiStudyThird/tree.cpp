#include "stdafx.h"
#include "tree.h"
#include <queue>
#include <fstream>

SearchTree::SearchTree()
	:rootPlayer(),startKyokumen()
{
	leave_branchNode = false;
	history.push_back(new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false)));
	nodecount = 0;
}

void SearchTree::set(const std::vector<std::string>& usitokens) {
	const auto moves = Move::usiToMoves(usitokens);
	set(Kyokumen(usitokens), moves);
}

void SearchTree::set(const Kyokumen& startpos,const std::vector<Move>& usihis) {
	if (!history.empty() && (history.size() <= usihis.size()) && startKyokumen == startpos)	{
		int i;
		for (i = 0; i < history.size() - 1; i++) {
			if (history[i+1ull]->move != usihis[i]) {
				goto makenewtree;
			}
		}
		for (; i < usihis.size(); i++) {
			SearchNode* root = getRoot();
			const Move nextmove = usihis[i];
			SearchNode* nextNode = nullptr;
			for (SearchNode* child : root->children) {
				if (child->move == nextmove) {
					nextNode = child;
					break;
				}
			}
			if (nextNode == nullptr) {
				nextNode = root->addChild(nextmove);
			}
			proceed(nextNode);
		}
		return;
	}
makenewtree:
	{
		if (!history.empty()) deleteTreeParallel(history.front(), history.size() - 1);
		history.clear();
		startKyokumen = startpos;
		SearchNode* rootNode = new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false));
		history.push_back(rootNode);
		rootPlayer = SearchPlayer(startKyokumen);
		for (auto& usimove : usihis) {
			SearchNode* next = rootNode->addChild(usimove);
			proceed(next);
		}
	}
}

SearchNode* SearchTree::getBestMove()const {
	SearchNode* const rootNode = getRoot();
	SearchNode* best = nullptr;
	double min = std::numeric_limits<double>::max();
	for (const auto child : rootNode->children) {
		const double eval = child->eval;
		if (eval <= min) {
			best = child;
			min = eval;
		}
	}
	return best;
}

std::vector<SearchNode*> SearchTree::getPV()const {
	SearchNode* node = getRoot();
	std::vector<SearchNode*> pv = { node };
	while (node != nullptr && !node->children.empty()) {
		SearchNode* best = nullptr;
		double min = std::numeric_limits<double>::max();
		for (const auto child : node->children) {
			const double eval = child->eval;
			if (eval <= min) {
				best = child;
				min = eval;
			}
		}
		node = best;
		pv.push_back(best);
	}
	return pv;
}

void SearchTree::proceed(SearchNode* node) {
	historymap.emplace(rootPlayer.kyokumen.getHash(), std::make_pair(rootPlayer.kyokumen.getBammen(), history.size()));
	rootPlayer.kyokumen.proceed(node->move);
	rootPlayer.feature.set(rootPlayer.kyokumen);
	deleteBranchParallel(getRoot(), node, history.size() - 1);
	history.push_back(node);
}

SearchNode* SearchTree::getRoot(unsigned threadNo, size_t increaseNodes) {
	std::lock_guard<std::mutex> lock(thmutex);
	lastRefRootByThread[threadNo] = history.size() - 1;
	nodecount += increaseNodes;
	if (search_enable/* && nodecount < nodesMaxCount*/) {//nodecountを減らす処理が書けてないので一時的に無効化している
		return getRoot();
	}
	else {
		return nullptr;
	}
}

void SearchTree::deleteBranchParallel(SearchNode* base, SearchNode* saved, uint8_t oldhisnum) {
	if (leave_branchNode) return;
	std::thread th([this,base,saved,oldhisnum]() 
		{
			bool immigrated;
			do {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				immigrated = true;
				std::lock_guard<std::mutex> lock(thmutex);
				for (uint8_t hnum : lastRefRootByThread) {
					if (hnum <= oldhisnum)
						immigrated = false;
				}
			} while (!immigrated);
			for (auto node : base->children) {
				if (node != saved) {
					const size_t delnum = node->deleteTree();
					nodecount -= delnum;
				}
			}
		}
	);
	th.detach();
}

void SearchTree::deleteTreeParallel(SearchNode* root,uint8_t oldhisnum) {
	std::thread th([this,root,oldhisnum]()
		{
			bool immigrated;
			do {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				immigrated = true;
				std::lock_guard<std::mutex> lock(thmutex);
				for (uint8_t hnum : lastRefRootByThread) {
					if (hnum == oldhisnum)
						immigrated = false;
				}
			} while (!immigrated);
			const size_t delnum = root->deleteTree();
			nodecount -= delnum;
			delete(root);
			nodecount--;
		}
	);
	th.detach();
}

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
		int st = static_cast<int>(node->state.load());
		fs << index << ", " << st << ", " << node->move.toUSI() << ", " << node->eval << ", " << node->mass << ", [";
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