#include "stdafx.h"
#include "tree.h"

SearchTree::SearchTree()
	:rootPlayer(),startKyokumen(),
	T_choice_quiescence(90),
	T_eval(40),T_depth(90),
	MassMax_QS(8)
{
	T_c_count = 0;
	setTchoice({ 30,60,90,120 });
	rootNode = new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false));
	nodecount = 0;
}

void SearchTree::set(const std::vector<std::string>& usitokens) {
	const auto moves = Move::usiToMoves(usitokens);
	set(Kyokumen(usitokens), moves);
}

void SearchTree::set(const Kyokumen& startpos,const std::vector<Move>& usihis) {
	if (!history.empty() && startKyokumen == startpos)	{
		int i;
		for (i = 0; i < history.size() - 1; i++) {
			if (history[i+1ull]->move != usihis[i]) {
				goto makenewtree;
			}
		}
		for (; i < usihis.size(); i++) {
			SearchNode* root = rootNode;
			const Move nextmove = usihis[i];
			SearchNode* nextNode = nullptr;
			for (SearchNode* child : root->children) {
				if (child->move == nextmove) {
					nextNode = child;
					break;
				}
			}
			if (nextNode == nullptr) {
				nextNode = new SearchNode(nextmove);
			}
			proceed(nextNode);
		}
		return;
	}
makenewtree:
	{
		if(!history.empty()) deleteTreeParallel(history.front());
		history.clear();
		startKyokumen = startpos;
		rootNode = new SearchNode(Move(koma::Position::NullMove, koma::Position::NullMove, false));
		history.push_back(rootNode);
		rootPlayer = SearchPlayer(startKyokumen);
		for (auto& usimove : usihis) {
			SearchNode* next = rootNode->addChild(usimove);
			proceed(next);
		}
	}
}

SearchNode* SearchTree::getBestMove()const {
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
	SearchNode* node = rootNode;
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
	rootPlayer.kyokumen.proceed(node->move);
	rootPlayer.feature.set(rootPlayer.kyokumen);
	history.push_back(node);
	deleteBranchParallel(rootNode, node);
	rootNode = node;
	thread_latestRootFlags.assign(thread_latestRootFlags.size(), false);
}

void SearchTree::setTchoice(const std::vector<double>& T) {
	const size_t size = T.size();
	for (size_t i = 0; i < T_choice.size(); i++) {
		T_choice[i] = T[i % size];
	}
}

double SearchTree::getTchoice() {
	const unsigned long long count = T_c_count++ % 64;
	return T_choice[count];
}

bool SearchTree::resisterLeafNode(SearchNode* const node) {
	std::lock_guard<std::mutex> lock(lnmutex);
	if (nmap.count(node) == 0) {//先約がいない場合
		nmap.insert(std::make_pair(node, 1));
		return true;
	}
	else { //先約がいる場合
		return false;
	}
}

void SearchTree::excludeLeafNode(SearchNode* const node) {
	std::lock_guard<std::mutex> lock(lnmutex);
	nmap.erase(node);
}

SearchNode* SearchTree::getRoot(unsigned threadNo, size_t increaseNodes) {
	std::lock_guard<std::mutex> lock(thmutex);
	thread_latestRootFlags[threadNo] = true;
	nodecount += increaseNodes;
	if (search_enable/* && nodecount < nodesMaxCount*/) {//nodecountを減らす処理が書けてないので一時的に無効化している
		return rootNode;
	}
	else {
		return nullptr;
	}
}

void SearchTree::deleteBranchParallel(SearchNode* base, SearchNode* saved) {
	std::thread th([this,base,saved]() 
		{
			bool immigrated;
			do {
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				immigrated = true;
				for (bool flag : thread_latestRootFlags) {
					if (!flag)
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

void SearchTree::deleteTreeParallel(SearchNode* root) {
	std::thread th([this,root]()
		{
			bool immigrated;
			do {
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				immigrated = true;
				for (bool flag : thread_latestRootFlags) {
					if (!flag)
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