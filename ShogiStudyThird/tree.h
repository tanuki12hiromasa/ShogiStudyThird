#pragma once
#include "node.h"
#include "kyokumen.h"
#include "evaluator.h"
#include <unordered_map>
#include <thread>
#include <queue>
#include <condition_variable>

class SearchTree {
public:
	SearchTree();
	~SearchTree();
	void reset();
	void set(const Kyokumen& startpos, const std::vector<Move>& moves);
	void set(const std::vector<std::string>& usitokens);
	void makeNewTree(const Kyokumen& startpos,const std::vector<Move>& moves);
	void makeNewTree(const std::vector<std::string>& usitokens);

	void addEvaluationCount(const uint64_t n) { evaluationcount += n; }

	SearchNode* getBestMove()const;//最もevalの高いrootのchildを返す
	std::vector<SearchNode*> getPV()const;//rootからのpvの連なりを返す
	void proceed(SearchNode* node);
	bool deleteGarbage();

	const uint64_t getEvaluationCount()const { return evaluationcount; }
	const std::vector<SearchNode*>& getHistory()const { return history; }
	const SearchPlayer& getRootPlayer()const { return rootPlayer; }
	std::pair<unsigned,SearchNode*> findRepetition(const Kyokumen& kyokumen)const;//過去に同一局面が無かったか検索する なければ-1を返す
	SearchNode* getRoot() const { return history.back(); }
	SearchNode* getGameRoot() const { return history.front(); }
	int getMoveNum() const { return history.size() - 1; }

	void foutTree()const;
private:

	std::unordered_multimap<std::uint64_t, std::pair<std::array<uint8_t, 95>, uint16_t>> historymap;
	std::vector<SearchNode*> history;
	Kyokumen startKyokumen;
	SearchPlayer rootPlayer;
	std::atomic_uint64_t evaluationcount;

	bool leave_branchNode = false;
	bool continuous_tree = true;

private:
	void addGarbage(SearchNode* const parent, bool deleteParent);

	std::queue<std::pair<SearchNode*, SearchNode::Children*>> garbage_parent;
	std::mutex mtx_deleteTrees;

	friend class Commander;
	friend class LearnCommander;
	friend class ShogiTest;
};
 
