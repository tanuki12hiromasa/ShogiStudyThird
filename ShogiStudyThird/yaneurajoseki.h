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
        bool on = false;
    };
    //定跡読み込み
    void readBook();
    //定跡利用
    std::string getBestMoveFromJoseki(std::string sfen);
private:
    bookNode getBestMove(std::string sfen);
    static std::string getSfenTrimed(std::string sfen);    //末尾の数字を取り除いたsfen列を返す
    //定跡を利用するか等の変数
    bool joseki_on = false;
    //定跡フォルダ名
    std::string joseki_folder_name = "joseki";
    std::string joseki_input_file_name = "user_book1.db";
    //定跡をランダムに選択するかどうか
    bool joseki_random = false;

    std::unordered_map<std::string, bookNode> bookJoseki;
};
