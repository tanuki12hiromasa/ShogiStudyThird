#pragma once
#include "node.h"
#include "kyokumen.h"
#include <iostream>
#include <fstream>
#include "usi.h"
#include <unordered_map>
#include "tree.h"
#include "josekiinput.h"
#include "josekioutput.h"
#include "josekioption.h"
#include "jdatabase.h"

class Joseki {
public:
    Joseki();

    //オプション設定
    JosekiOption option;

    void setOption(std::vector<std::string>tokens);
    void coutOption();

    JosekiInput input;
    JosekiOutput output;
    JosekiDataBase josekiDataBase;

private:

};
