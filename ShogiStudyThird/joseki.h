#pragma once
#include "node.h"
#include "kyokumen.h"
#include <iostream>
#include <fstream>
#include "usi.h"
#include <unordered_map>

class Joseki {
    //共通
public:
private:
    //保存するノード
    struct josekinode {
        size_t index = -1;
        SearchNode::State st = SearchNode::State::N;
        uint16_t move = -1;
        double mass = 0;
        double eval = 0;
        size_t childCount = -1;
        size_t childIndex = -1;
        josekinode(size_t _index, SearchNode::State _st, uint16_t _move, double _mass, double _eval, int _childCount, size_t _childIndex) :index(_index), st(_st), move(_move), mass(_mass), eval(_eval), childCount(_childCount), childIndex(_childIndex) {

        }
        josekinode() {

        }
        void viewNode() {
            std::cout
                << "index:" << index
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


    //出力
public:
    void josekiOutput(SearchNode* root,const Kyokumen rootKyokumen);    //定跡書き出し
    SearchNode* getJosekiNodes()const { return josekiNodes;}            //
    Kyokumen getKyokumen()const { return kyokumen; }
    size_t getChildCount()const { return childCount; }

    void setOutputFileName(std::string filename) {
        this->outputFileName = filename + ".bin";
        this->outputFileInfoName = filename + "_info.txt";
    }
private:
    std::string outputFileName = "treejoseki.bin";
    std::string outputFileInfoName = "treejoseki_info.txt";

    //入力
public:
    void josekiInput();
    void setInputFileName(std::string filename) {
        this->inputFileName = filename + ".bin";
        this->inputFileInfoName = filename + "_info.txt";
    }

private:
    struct inputfile {  //読み込む定跡木の情報を格納
        FILE* fp;
        SearchNode** nodes;
        size_t* parents;
        std::string sfen;
        fpos_t firstPos;
        size_t nodeCount;
        josekinode *josekiNodes;
        void open(std::string filename,std::string fileinfoname) {
            std::ifstream ifs(fileinfoname);
            std::string nodeCountStr;
            std::getline(ifs, nodeCountStr);
            std::getline(ifs, sfen);
            ifs.close();
            nodeCount = std::stol(usi::split(nodeCountStr, ',')[1]);    //infoファイルからノード数を取得

            errno_t err = fopen_s(&fp, (filename).c_str(), "rb");
            fgetpos(fp, &firstPos);
            
            nodes = (SearchNode**)malloc(sizeof(SearchNode*) * nodeCount);
            parents = (size_t*)malloc(sizeof(size_t) * nodeCount);
            josekiNodes = (josekinode*)malloc(sizeof(josekinode) * nodeCount);
            fread(josekiNodes, sizeof(josekinode), nodeCount, fp);
        }
        void close() {
            free(nodes);
            free(parents);
            fclose(fp);
        }
    } inputFile;
    
    SearchNode* josekiNodes;
    Kyokumen kyokumen;
    size_t childCount;

    std::vector<size_t> yomikomiLine(inputfile &inputFile, const size_t index);
    void yomikomiRecursive(inputfile &inputFile, const size_t index);
    std::vector<size_t> yomikomiDepth(inputfile &inputFile, const size_t index, const int depth);
    void yomikomiBreath(inputfile& inputFile, const size_t index);

    std::string inputFileName = "treejoseki.bin";       //定跡木を格納しておくファイル
    std::string inputFileInfoName = "treejoseki_info.txt";   //定跡木の情報を格納しておくファイル


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
