#pragma once
#include "nnue_feature.h"
#include "../player.h"

namespace Eval::NNUE {
	using SearchPlayer = Player<NNUE_feat, Cache>;

	// Hash value of evaluation function structure
// 評価関数の構造のハッシュ値
	constexpr std::uint32_t kHashValue = NNUE_feat::GetHashValue() ^ Network::GetHashValue();


	class NNUE_evaluator {
	private:
		static std::unique_ptr<Network> network;
		static std::string ifolderpath;
		static std::string ofolderpath;
	public:
		static bool ReadHeader(std::istream& stream, std::uint32_t* hash_value, std::string* architecture);
		static bool WriteHeader(std::ostream& stream, std::uint32_t hash_value, const std::string& architecture);
		static bool ReadParameters(std::istream& stream);
		static bool WriteParameters(std::ostream& stream);



		static void init();
		static void save();
		static double evaluate(const SearchPlayer& player);
	};
}