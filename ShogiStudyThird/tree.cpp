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
	oldrootNode = rootNode;
	nodecount = 0;
}

void SearchTree::set(const Kyokumen& startpos,const std::vector<Move>& usihis) {
	if (startKyokumen != startpos)
		goto makenewtree;
	{
		Kyokumen usihisKyokumen(startpos);

	}


makenewtree:
	{
		std::thread th(&SearchTree::deleteTree, this, history.front());
		th.detach();
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

void SearchTree::proceed(SearchNode* node) {
	rootPlayer.proceed(node->move);
	history.push_back(node);
	rootNode = node;
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