#pragma once
#include "node.h"
#include "kyokumen.h"
#include <iostream>
#include <fstream>
#include "usi.h"
#include <unordered_map>
#include "tree.h"
#include "josekioption.h"

class YaneuraJoseki {
public:

    YaneuraJoseki();
    //オプション設定
    JosekiOption option;
    
    //やねうら王形式の定跡を読み込んで利用する
    struct bookNode {
        Move bestMove;  //盤面におけるベストな指し手
        int depth;
        bool on = false;
    };
    //定跡読み込み
    void readBook();
    //定跡利用
    std::string getBestMoveFromJoseki(std::string sfen);

    void outputJosekiAsYaneura(SearchNode* root, std::string outputFileName, int maxCount) {
        std::string s = "#YANEURAOU-DB2016 1.00\n";
        std::vector<std::string> startpos = { "position","startpos" };
        Kyokumen kyo = Kyokumen(startpos);
        outputJosekiAsYaneura(root, kyo, &s, 1, maxCount);
        std::ofstream ofs(outputFileName);
        ofs << s;
        std::cout << "end" << std::endl;
        ofs.close();
    }

    void outputJosekiAsYaneura(SearchNode* node, Kyokumen kyokumen, std::string* st, int depth, int maxCount);
    size_t getYaneuraJosekiCount() { return bookJoseki.size(); }
    bookNode getBestMove(std::string sfen);
private:
    static std::string getSfenTrimed(std::string sfen);    //末尾の数字を取り除いたsfen列を返す

    std::unordered_map<std::string, bookNode> bookJoseki;

    int lastDepth = -1;
};
