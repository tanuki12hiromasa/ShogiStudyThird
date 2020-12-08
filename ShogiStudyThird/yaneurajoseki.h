#pragma once
#include "node.h"
#include "kyokumen.h"
#include <iostream>
#include <fstream>
#include "usi.h"
#include <unordered_map>
#include "tree.h"

class YaneuraJoseki {
public:
    //オプション設定
    void setOption(std::vector<std::string>tokens);
    //オプション描画
    static void coutOption();

    //定跡がオンになってるか取得。なってなかったら以降は定跡を使わない
    inline bool getJosekiOn() {
        return joseki_on;
    }

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

    void outputJosekiAsYaneura(SearchNode* node, Kyokumen kyokumen, std::string* st, int depth, int maxCount) {
        if (node->children.size() == 0) {
            return;
        }
        if (node->move.toUSI() != "nullmove" && node->move.toUSI() != "1a1a") {
            kyokumen.proceed(node->move);
        }
        std::string sfen = kyokumen.toSfen();
        std::vector<std::string> ts = usi::split(sfen, ' ');
        ts[ts.size() - 1] = std::to_string(depth);
        //ts[ts.size() - 3] = depth % 2 == 0 ? "b" : "w";
        sfen = "";
        for (int i = 0; i < ts.size(); ++i) {
            if (i != 0) {
                sfen += " ";
            }
            sfen += ts[i];
        }
        *st += sfen + "\n";
        int size = node->children.size();
        int maxSize = maxCount;
        //int count = (node->children.size()/* > 0*/);
        int count = (size > maxSize ? maxSize : size);
        for (int i = 0; i < count; ++i) {
            SearchNode* child = node->children[i];
            std::string move = child->move.toUSI();
            std::string next = "none";
            if (child->children.size() > 0) {
                next = child->children[0]->move.toUSI();
            }
            std::string eval = std::to_string(int(child->eval + 0.5));
            std::string depth = std::to_string(int(child->mass + 0.5));
            std::string selected = "0";
            *st += move + " " + next + " " + eval + " " + depth + " " + selected + " " + "\n";
        }

        for (int i = 0; i < count/*node->children.size()*/; ++i) {
            SearchNode* child = node->children[i];
            outputJosekiAsYaneura(child, kyokumen, st, depth + 1, maxCount);
        }
        //std::cout << *st << std::endl;
    }
    size_t getYaneuraJosekiCount() { return bookJoseki.size(); }
    bookNode getBestMove(std::string sfen);
private:
    static std::string getSfenTrimed(std::string sfen);    //末尾の数字を取り除いたsfen列を返す
    //定跡を利用するか等の変数
    bool joseki_on = false;
    //定跡フォルダ名
    std::string joseki_folder_name = "joseki";
    std::string joseki_input_file_name = "user_book1.db";
    //定跡をランダムに選択するかどうか
    bool joseki_random = false;

    std::unordered_map<std::string, bookNode> bookJoseki;

    int lastDepth = -1;
};
