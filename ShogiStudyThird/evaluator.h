#pragma once
#include "apery_evaluate.h"

//ここで使用する評価関数を指定する
/*
差分なしの場合
Evaluatorクラスは、必ずinit,evaluateを持つようにする
static void init(); 評価関数の初期化
static void evaluate(EvaluatedNodes_full& en);EvaluatedNodes_fullを受け取り各ノードの評価値を計算しen.nodesに格納する

差分計算ありの場合 (evaluated_node.hで #define EVAL_DIFF_ONLY とすること)
EvaluatorとFeatureを用意する
Featureはfeature.hで
Evaluatorはinit,evaluateを持つ
static void init();
static void evaluate(EvaluatedNodes_diff& en, const Feature& feat);
Featureはset,proceed,引数なしのデフォルトコンストラクタを持つ
void set(const Kyokumen& kyokumen);
void proceed(const Kyokumen& before, const Move& move);
*/

//bonanza6.0
//using Evaluator = bonanza::bona_eval;

//newEval -芝浦将棋softmax
//using Evaluator = sss::sss_eval;

//apery
using Evaluator = apery::apery_evaluator;
