#pragma once
#include "node.h"
#include "kyokumen.h"
#include "evaluator.h"
#include "player.h"
#include <unordered_map>
#include <fstream>

class SearchTree {
public:
	SearchTree();
	void set(const Kyokumen& startpos,const std::vector<Move>& moves);
	void set(const std::vector<std::string>& usitokens);

	void setNodeMaxsize(const size_t s) { nodesMaxCount = s; }

	void permitSearch() { search_enable = true; }
	void prohibitSearch() { search_enable = false; }
	SearchNode* getBestMove()const;//最もevalの高いrootのchildを返す
	std::vector<SearchNode*> getPV()const;//rootからのpvの連なりを返す
	void proceed(SearchNode* node);
	void deleteBranchParallel(SearchNode* base, SearchNode* saved, uint8_t oldhisnum);//baseのsaved以下以外の探索木を子ノードを残して消去する

	const uint64_t getNodeCount() const { return nodecount; }
	const std::vector<SearchNode*>& getHistory()const { return history; }
	const SearchPlayer& getRootPlayer()const { return rootPlayer; }
	std::pair<unsigned,SearchNode*> findRepetition(const Kyokumen& kyokumen)const;//過去に同一局面が無かったか検索する なければ-1を返す
	SearchNode* getRoot(unsigned threadNo, size_t increaseNodes);
	SearchNode* getRoot() const { return history.back(); }

	void logBM();
	void foutTree()const;
private:
	void deleteTreeParallel(SearchNode* root,uint8_t oldhisnum);//rootを含め子孫を全消去する

	std::unordered_multimap<std::uint64_t, std::pair<std::array<uint8_t, 95>, uint16_t>> historymap;
	std::vector<SearchNode*> history;
	Kyokumen startKyokumen;
	SearchPlayer rootPlayer;
	std::atomic_uint64_t nodecount;
	std::uint64_t nodesMaxCount;

	bool leave_branchNode;
	std::vector<std::uint8_t> lastRefRootByThread;
	std::atomic_bool search_enable;
	std::mutex thmutex;
	std::mutex logmutex;
	std::ofstream logstream;

	friend class Commander;
	friend class ShogiTest;
};
 
