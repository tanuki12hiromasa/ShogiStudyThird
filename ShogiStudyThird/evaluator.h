#pragma once
#include "player.h"

#include "apery_evaluate.h"
#include "apery_feature.h"
#include "nnue/nnue_evaluate.h"

//使用する評価関数を指定する
//#define USE_KPPT
#define USE_NNUE

//bonanza6.0
//using Evaluator = bonanza::bona_eval;

//newEval -芝浦将棋softmax
//using Evaluator = sss::sss_eval;
//using Feature = sss::sss_feat;

//apery
#ifdef USE_KPPT
using Evaluator = apery::apery_evaluator;
using Feature = apery::apery_feat;
using FeatureCache = apery::EvalSum;
#endif

//NNUE
#ifdef USE_NNUE
using Evaluator = Eval::NNUE::NNUE_evaluator;
using Feature = Eval::NNUE::NNUE_feat;
using FeatureCache = Eval::NNUE::Cache;
#endif

using SearchPlayer = Player<Feature, FeatureCache>;
