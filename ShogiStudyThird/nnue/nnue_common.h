#pragma once
#include <cstdint>
#include <cstddef>
#include <intrin.h>
#include <iostream>
#include <string>
#define USE_AVX2

namespace Eval {

	namespace NNUE {

		// 評価関数ファイルのバージョンを表す定数
		constexpr ::std::uint32_t kVersion = 0x7AF32F16u;

		// 評価値の計算で利用する定数
		constexpr int FV_SCALE = 16;
		constexpr int kWeightScaleBits = 6;

		// キャッシュラインのサイズ（バイト単位）
		constexpr std::size_t kCacheLineSize = 64;

		// SIMD幅（バイト単位）
#if defined(USE_AVX2)
		constexpr std::size_t kSimdWidth = 32;
#elif defined(USE_SSE2)
		constexpr std::size_t kSimdWidth = 16;
#elif defined(IS_ARM)
		constexpr std::size_t kSimdWidth = 16;
#endif
		constexpr std::size_t kMaxSimdWidth = 32;

		// 変換後の入力特徴量の型
		using TransformedFeatureType = ::std::uint8_t;

		// インデックスの型
		using IndexType = ::std::uint32_t;

		// 学習用クラステンプレートの前方宣言
		template <typename Layer>
		class Trainer;

		// n以上で最小のbaseの倍数を求める
		template <typename IntType>
		constexpr IntType CeilToMultiple(IntType n, IntType base) {
			return (n + base - 1) / base * base;
		}

// read_little_endian() is our utility to read an integer (signed or unsigned, any size)
// from a stream in little-endian order. We swap the byte order after the read if
// necessary to return a result with the byte ordering of the compiling machine.
		template <typename IntType>
		inline IntType read_little_endian(std::istream& stream) {

			IntType result;
			std::uint8_t u[sizeof(IntType)];
			typename std::make_unsigned<IntType>::type v = 0;

			stream.read(reinterpret_cast<char*>(u), sizeof(IntType));
			for (std::size_t i = 0; i < sizeof(IntType); ++i)
				v = (v << 8) | u[sizeof(IntType) - i - 1];

			std::memcpy(&result, &v, sizeof(IntType));
			return result;
		}
	}  // namespace NNUE

}  // namespace Eval
