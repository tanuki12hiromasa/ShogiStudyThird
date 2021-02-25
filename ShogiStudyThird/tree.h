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
	void set(const Kyokumen& startpos, const std::vector<Move>& moves);
	void set(const std::vector<std::string>& usitokens);
	void makeNewTree(const Kyokumen& startpos,const std::vector<Move>& moves);
	void makeNewTree(const std::vector<std::string>& usitokens);

	void setNodeMaxsize(const size_t s) { nodesMaxCount = s; }
	void addEvaluationCount(const uint64_t n) { evaluationcount += n; }

	SearchNode* getBestMove()const;//最もevalの高いrootのchildを返す
	std::vector<SearchNode*> getPV()const;//rootからのpvの連なりを返す
	void proceed(SearchNode* node);

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
	std::uint64_t nodesMaxCount;

	bool leave_branchNode = false;
	bool continuous_tree = true;

private:
	SearchNode* addNewChild(SearchNode* const parent, const Move& move);
	void deleteTrees(SearchNode::Children* root);
	void deleteTreesLoop();

	std::thread thread_deleteTrees;
	std::queue<SearchNode::Children*> roots_deleteTrees;
	std::condition_variable cv_deleteTrees;
	std::mutex mtx_deleteTrees;
	std::atomic_bool enable_deleteTrees;

	friend class Commander;
	friend class ShogiTest;
};
 
