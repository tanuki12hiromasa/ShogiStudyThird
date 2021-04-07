#pragma once
#include "tree.h"
#include "josekinode.h"
#include "josekioption.h"
#include "jdatabase.h"

class JosekiInput {
    //入力
public:
    JosekiInput();
    void josekiInput(SearchTree* tree, size_t firstIndex = 0);
    //定跡に保存された最善手を返す
    bool getBestMove(SearchTree* tree, std::vector<SearchNode*> history);
    //定跡の最善手指しががまだ続いてる
    bool getBestMoveOn() { return bestMoveOn; }
    JosekiOption option;
    void init();
private:
    FILE* fp;	//定跡本体の読み込み用ファイル
    SearchNode* nodesForProgram;	//実際にプログラム内で活用していく定跡が格納されるところ
    size_t* parentsIndex;	//ノードのインデックスと親ノードの対応
    std::vector<std::string> tokenOfPosition;	//定跡が指し示す局面
    josekinode* nodesFromFile;	//定跡本体から読みこんだ定跡を収めるところ
    bool bestMoveOn;
    SearchNode* dummy = new SearchNode;

    std::vector<size_t> yomikomiLine(const size_t index);
    void yomikomiRecursive(const size_t index);
    std::vector<size_t> yomikomiDepth(const size_t index, const int depth);
};
