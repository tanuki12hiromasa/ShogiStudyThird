#pragma once
#include "tree.h"
#include "josekinode.h"
#include "josekioption.h"
#include "jdatabase.h"

class JosekiOutput {
    //出力
public:
    JosekiOutput();
    //定跡書き出し
    void josekiOutput(const std::vector<SearchNode*> const history);
    //末端からのバックアップ
    void backUp(std::vector<SearchNode*> history);
    //データベースへ書き出し
    void josekiOutputToDataBaseWithPath(JosekiDataBase* jdb, SearchNode* node, std::string path);
    void josekiOutputToDataBaseWithParent(JosekiDataBase* jdb, SearchNode* node, int parentId);
    JosekiOption option;
private:
    //情報ファイルへの書き出し
    bool outputInfo(const std::vector<SearchNode*> const history);
};
