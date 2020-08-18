#pragma once
#include "node.h"
#include "kyokumen.h"
#include "evaluator.h"
#include "player.h"
#include <unordered_map>

class SearchTree {
public:
	SearchTree();
	std::pair<bool, std::vector<SearchNode*>> set(const Kyokumen& startpos,const std::vector<Move>& moves);//返値は探索木を使えればtrue 作り直したらfalse
	std::pair<bool, std::vector<SearchNode*>> set(const std::vector<std::string>& usitokens);
	void makeNewTree(const Kyokumen& startpos,const std::vector<Move>& moves);
	void makeNewTree(const std::vector<std::string>& usitokens);

	void setNodeMaxsize(const size_t s) { nodesMaxCount = s; }
	void addNodeCount(const size_t n) { nodecount += n; }

	SearchNode* getBestMove()const;//最もevalの高いrootのchildを返す
	std::vector<SearchNode*> getPV()const;//rootからのpvの連なりを返す
	void proceed(SearchNode* node);
	void deleteBranch(SearchNode* base, const std::vector<SearchNode*>& savedNodes);//baseのsaved以下以外の探索木を子ノードを残して消去する
	void deleteTree(SearchNode* const root);//rootを含め子孫を全消去する

	const uint64_t getNodeCount() const { return nodecount; }
	const std::vector<SearchNode*>& getHistory()const { return history; }
	const SearchPlayer& getRootPlayer()const { return rootPlayer; }
	std::pair<unsigned,SearchNode*> findRepetition(const Kyokumen& kyokumen)const;//過去に同一局面が無かったか検索する なければ-1を返す
	SearchNode* getRoot() const { return history.back(); }

	void foutTree()const;
	void foutJoseki(int joseki_make_type,int fileCount)const;
	void foutJoseki()const;
	void foutJosekiBin(int joseki_make_type, int fileCount)const;

	void setRoot(SearchNode* const root, const Kyokumen& kyokumen, size_t nodes);
	void setRoot(SearchNode* const root, std::vector<std::string> tokens, size_t nodes);
private:

	std::unordered_multimap<std::uint64_t, std::pair<std::array<uint8_t, 95>, uint16_t>> historymap;
	std::vector<SearchNode*> history;
	Kyokumen startKyokumen;
	SearchPlayer rootPlayer;
	std::atomic_uint64_t nodecount;
	std::uint64_t nodesMaxCount;

	bool leave_branchNode;

	friend class Commander;
	friend class ShogiTest;
};
 
