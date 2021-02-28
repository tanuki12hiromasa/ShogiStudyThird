#pragma once
#include "node.h"

class Pruning {
    //枝刈り
public:
    //指定されたノードから下を全て枝刈りする。返り値は刈ったノードの数
    size_t pruning(SearchNode* root);
private:
    //指定されたノードに対して再帰的に枝刈りを行う
    size_t partialPruning(SearchNode* node, std::vector<SearchNode*>history, double select = -1, int depth = 10, double backupRate = 1);
    //実際の枝刈り処理を行う
    size_t pruningExecuter(SearchNode* node, std::vector<SearchNode*>history);
    //枝刈りを行うものの設定を呼び出す関数
    bool isPruning(SearchNode* node, double select = 1, int depth = 10, double backupRate = 1);
    ////深さバックアップ温度再計算用の温度
    //double T_d = 100;
    double pruningBorder = 0.1;
    double pruningBorderEval = 50000;
    bool pruning_on = false;
    //枝刈りのタイプ。0は実現確率
    int pruning_type = 0;
    //枝刈りをしない深さ
    int pruning_depth = 5;
    double pruning_T_c = 40;
    //枝刈り時に残す残すノードの数(上位ノード)
    int leaveNodeCount = 0;

};
