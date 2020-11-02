#pragma once
#include "apery_feature.h"
#include "evaluated_nodes.h"
#include "player.h"

namespace apery {
	using SearchPlayer = Player<apery_feat, EvalSum>;
	class apery_evaluator {
	public:
		static void init() { init(ifolderpath); }
		static void init(const std::string& path);
		static void save() { save(ofolderpath); }
		static void save(const std::string& path);
		static void evaluate(EvaluatedNodes_full& en);
		static void evaluate(EvaluatedNodes_diff& en, const apery_feat& pfeat);
		static void evaluate(std::vector<SearchNode*> nodes, const SearchPlayer& parent_player);
		static void evaluate(SearchNode* node, const SearchPlayer& player);//playerはnode自身のを渡す
		static double evaluate(const SearchPlayer& player);

		static void setpath_input(const std::string& path) { ifolderpath = path; }
		static void setpath_output(const std::string& path) { ofolderpath = path; }

		static void genInitalPram(const std::string& folderpath);
	private:
		static std::string ifolderpath;
		static std::string ofolderpath;
	};
}