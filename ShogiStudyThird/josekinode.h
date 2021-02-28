#pragma once
#include "node.h"

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
