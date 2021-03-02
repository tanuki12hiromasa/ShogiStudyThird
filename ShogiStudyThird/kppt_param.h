#pragma once
#include "kyokumen.h"

namespace kppt {
	constexpr int SquareNum = 81;
	inline constexpr int inverse(int pos) { return SquareNum - 1 - pos; }

	constexpr int FVScale = 32;
	constexpr int ScoreZero = 0;
	constexpr int PawnScore = (100 * 9 / 10);
	constexpr int LanceScore = (350 * 9 / 10);
	constexpr int KnightScore = (450 * 9 / 10);
	constexpr int SilverScore = (550 * 9 / 10);
	constexpr int GoldScore = (600 * 9 / 10);
	constexpr int BishopScore = (950 * 9 / 10);
	constexpr int RookScore = (1100 * 9 / 10);
	constexpr int ProPawnScore = (600 * 9 / 10);
	constexpr int ProLanceScore = (600 * 9 / 10);
	constexpr int ProKnightScore = (600 * 9 / 10);
	constexpr int ProSilverScore = (600 * 9 / 10);
	constexpr int HorseScore = (1050 * 9 / 10);
	constexpr int DragonScore = (1550 * 9 / 10);
	constexpr int KingScore = (15000);

	const std::array<int, static_cast<size_t>(koma::Koma::KomaNum)> PieceScoreArr = {
		PawnScore, LanceScore, KnightScore, SilverScore, BishopScore, RookScore, GoldScore,
		ScoreZero, // King
		ProPawnScore, ProLanceScore, ProKnightScore, ProSilverScore, HorseScore, DragonScore,
		-PawnScore, -LanceScore, -KnightScore, -SilverScore, -BishopScore, -RookScore, -GoldScore,
		ScoreZero, // King
		-ProPawnScore, -ProLanceScore, -ProKnightScore, -ProSilverScore, -HorseScore, -DragonScore,
	};
	inline int PieceScore(koma::Koma k) { return PieceScoreArr[static_cast<size_t>(k)]; }
	inline int PieceScore(koma::Mochigoma m) { return PieceScoreArr[static_cast<size_t>(m)]; }
	inline int PieceScore(koma::Mochigoma m, bool teban) { return teban ? PieceScoreArr[static_cast<size_t>(m)] : -PieceScoreArr[static_cast<size_t>(m)]; }

	enum EvalIndex : int32_t {
		f_hand_pawn = 0, // 0
		e_hand_pawn = f_hand_pawn + 19,
		f_hand_lance = e_hand_pawn + 19,
		e_hand_lance = f_hand_lance + 5,
		f_hand_knight = e_hand_lance + 5,
		e_hand_knight = f_hand_knight + 5,
		f_hand_silver = e_hand_knight + 5,
		e_hand_silver = f_hand_silver + 5,
		f_hand_gold = e_hand_silver + 5,
		e_hand_gold = f_hand_gold + 5,
		f_hand_bishop = e_hand_gold + 5,
		e_hand_bishop = f_hand_bishop + 3,
		f_hand_rook = e_hand_bishop + 3,
		e_hand_rook = f_hand_rook + 3,
		fe_hand_end = e_hand_rook + 3,

		f_pawn = fe_hand_end,
		e_pawn = f_pawn + 81,
		f_lance = e_pawn + 81,
		e_lance = f_lance + 81,
		f_knight = e_lance + 81,
		e_knight = f_knight + 81,
		f_silver = e_knight + 81,
		e_silver = f_silver + 81,
		f_gold = e_silver + 81,
		e_gold = f_gold + 81,
		f_bishop = e_gold + 81,
		e_bishop = f_bishop + 81,
		f_horse = e_bishop + 81,
		e_horse = f_horse + 81,
		f_rook = e_horse + 81,
		e_rook = f_rook + 81,
		f_dragon = e_rook + 81,
		e_dragon = f_dragon + 81,
		fe_end = e_dragon + 81
	};
	inline constexpr EvalIndex operator+(const EvalIndex lhs, const int32_t rhs) { return static_cast<EvalIndex>(static_cast<int32_t>(lhs) + rhs); }

	const std::array<EvalIndex, static_cast<size_t>(koma::Koma::KomaNum)> komaToIndexArr = {
		f_pawn, f_lance, f_knight, f_silver, f_bishop, f_rook, f_gold, fe_end,
		f_gold, f_gold,  f_gold,   f_gold,   f_horse,  f_dragon,
		e_pawn, e_lance, e_knight, e_silver, e_bishop, e_rook, e_gold, fe_end,
		e_gold, e_gold,  e_gold,   e_gold,   e_horse,  e_dragon
	};
	inline EvalIndex komaToIndex(koma::Koma k) { return komaToIndexArr[static_cast<size_t>(k)]; }
	const std::array<EvalIndex, static_cast<size_t>(koma::Mochigoma::MochigomaNum)> sMochiToIndexArr = {
		f_hand_pawn,f_hand_lance,f_hand_knight,f_hand_silver,f_hand_bishop,f_hand_rook,f_hand_gold
	};
	const std::array<EvalIndex, static_cast<size_t>(koma::Mochigoma::MochigomaNum)> gMochiToIndexArr = {
		e_hand_pawn,e_hand_lance,e_hand_knight,e_hand_silver,e_hand_bishop,e_hand_rook,e_hand_gold
	};
	inline EvalIndex mochiToIndex(koma::Mochigoma k, bool teban) { return teban ? sMochiToIndexArr[static_cast<size_t>(k)] : gMochiToIndexArr[static_cast<size_t>(k)]; }
	constexpr EvalIndex mirror(const EvalIndex index) {
		if (index < fe_hand_end)return index;
		int pos = (index - fe_hand_end) % 81;
		return static_cast<EvalIndex>((int)index - pos + koma::mirrorX(pos));
	}
}