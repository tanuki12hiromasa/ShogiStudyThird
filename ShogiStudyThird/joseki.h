﻿#pragma once
#include "node.h"
#include "kyokumen.h"
#include <iostream>
#include <fstream>
#include "usi.h"
#include <unordered_map>
#include "tree.h"

class Joseki {
    //共通
public:
    void setOption(std::vector<std::string>tokens);
    void printOption();
    bool getYomikomiOn() { return yomikomi_on; }
private:
    //定跡を利用するか等の変数
    bool yomikomi_on;
    //対局終了時に書き出すかどうかの変数
    bool kakidashi_on;
    //複数回対局を行って拡大させるかどうかの変数
    bool joseki_loop = false;
    std::string loopFileName = "joseki\\next.txt";
    //ループ用のファイルの中身を増減させる。返り値は増減前
    int nextAddForJosekiLoop() {
        return nextForJosekiLoop(1);
    }
    int nextSubForJosekiLoop() {
        return nextForJosekiLoop(-1);
    }
    int nextForJosekiLoop(int add) {
        std::ifstream ifs(loopFileName);
        int r = 0;
        if (ifs.is_open()) {
            ifs >> r;
            ifs.close();
        }

        std::ofstream ofs(loopFileName);
        ofs << r + add << std::endl;
        ofs.close();
        return r;
    }

    //保存するノード
    struct josekinode {
        //size_t index = -1;
        SearchNode::State st = SearchNode::State::N;
        uint16_t move = -1;
        double mass = 0;
        double eval = 0;
        int childCount = -1;
        size_t childIndex = -1;
        josekinode(size_t _index, SearchNode::State _st, uint16_t _move, double _mass, double _eval, int _childCount, size_t _childIndex) :/*index(_index),*/ st(_st), move(_move), mass(_mass), eval(_eval), childCount(_childCount), childIndex(_childIndex) {

        }
        josekinode() {

        }
        void viewNode() {
            std::cout
                //<< "index:" << index
                << " state:" << (int)st
                << " move:" << move
                << " mass:" << mass
                << " eval:" << eval
                << " childCount:" << childCount
                << " childIndex:" << childIndex
                << std::endl;
        }
    };
    
    //時間計測用
    time_t startTime;
    std::string timerStart() {
        startTime = clock();
        time_t now = time(NULL);
        struct tm pnow;
        localtime_s(&pnow, &now);
        return "開始時刻　" + std::to_string(pnow.tm_hour) + "時" + std::to_string(pnow.tm_min) + "分" + std::to_string(pnow.tm_sec) + "秒";
    }
    std::string timerInterval() {
        return std::to_string((clock() - startTime) / (double)CLOCKS_PER_SEC);
    }

    std::string folderName = "joseki\\";

    //出力
public:
    //定跡書き出し
    void josekiOutput(const std::vector<SearchNode*> const history);
    //定跡書き出し判定付きの書き出し
    void josekiOutputIGameOver(const std::vector<SearchNode*> const history,std::vector<std::string> tokens);
    //定跡をテキスト形式で書き出し。人の目で見てわかりやすいように。
    void josekiTextOutput(const std::vector<SearchNode*> const history);
    SearchNode* getJosekiNodes()const { return nodesForProgram[0];}
    Kyokumen getKyokumen()const { return kyokumen; }
    size_t getChildCount()const { return childCount; }
    //末端からのバックアップ
    void backUp(std::vector<SearchNode*> history);

    void setOutputFileName(std::string filename) {
        std::string add = "";
        if (joseki_loop) {
            int num = nextForJosekiLoop(0);
            add = std::to_string(num);
        }

        this->outputFileName = folderName + filename + add + ".bin";
        this->outputFileInfoName = folderName + filename + add + "_info.txt";
    }
private:
    std::string outputFileName = "joseki\\defaultjoseki_output.bin";
    std::string outputFileInfoName = "joseki\\defaultjoseki_output_info.txt";
    std::string result = "none";
    
    double backup_T_e = 1;
    double backup_T_d = 1;

    //入力
public:
    void josekiInput(SearchTree* tree);
    void setInputFileName(std::string filename) {
        std::string add = "";
        if (true) {
            int num = nextAddForJosekiLoop();
            add = std::to_string(num);
        }
        this->inputFileName = folderName + filename + add + ".bin";
        this->inputFileInfoName = folderName + filename + add + "_info.txt";
    }

private:
    FILE* fp;	//定跡本体の読み込み用ファイル
    SearchNode** nodesForProgram;	//実際にプログラム内で活用していく定跡が格納されるところ
    size_t* parentsIndex;	//ノードのインデックスと親ノードの対応
    std::vector<std::string> tokenOfPosition;	//定跡が指し示す局面
    size_t nodeCount;	//ノード数
    josekinode* nodesFromFile;	//定跡本体から読みこんだ定跡を収めるところ


    SearchNode* root;
    Kyokumen kyokumen;
    size_t childCount;

    std::vector<size_t> yomikomiLine(const size_t index);
    void yomikomiRecursive(const size_t index);
    std::vector<size_t> yomikomiDepth(const size_t index, const int depth);
    void yomikomiBreath(const size_t index);

    std::string inputFileName = "joseki\\defaultjoseki_input.bin";       //定跡木を格納しておくファイル
    std::string inputFileInfoName = "joseki\\defaultjoseki_input_info.txt";   //定跡木の情報を格納しておくファイル

    //枝刈り
public:
    //指定されたノードから下を全て枝刈りする。返り値は刈ったノードの数
    size_t pruning(SearchNode* root);
private:
    //指定されたノードに対して再帰的に枝刈りを行う
    size_t partialPruning(SearchNode* node, std::vector<SearchNode*>history,double select = -1);
    //実際の枝刈り処理を行う
    size_t pruningExecuter(SearchNode* node, std::vector<SearchNode*>history);
    //枝刈りを行うものの設定を呼び出す関数
    bool isPruning(SearchNode* node, double select = 1);
    ////深さバックアップ温度再計算用の温度
    //double T_d = 100;
    double pruningBorder = 0.1;
    bool pruning_on = false;
    //枝刈りのタイプ。0は実現確率
    int pruning_type = 0;

    //従来の定跡を読み込んで利用する
public:
    struct bookNode {
        Move bestMove;  //盤面におけるベストな指し手
        Move nextMove;  //相手の最適な指し手
        double value = 0;   //評価値
        double depth = -1;   //深さ
        int num = -1;   //出現回数
    }; 
    void readBook(std::string fileName);
    bookNode getBestMove(std::string sfen);
    static std::string getSfenTrimed(std::string sfen);    //末尾の数字を取り除いたsfen列を返す
private:
    std::unordered_map<std::string,bookNode> bookJoseki;
};