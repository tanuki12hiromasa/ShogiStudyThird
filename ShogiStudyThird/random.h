#pragma once
#include <cstdint>
#include <array>

namespace Random {
	using u64 = std::uint64_t;

	/*
	xorshift256+ という高速な乱数生成器
	[0,1]の浮動小数点で利用する前提になっていて、
	下位ビットの精度を低くする代わりに生成速度を高速化している
	整数として使用する場合はxorshift256++等を使用する
	引用元： https://prng.di.unimi.it/ 
	*/
	class xoshiro256p {
	public:
		xoshiro256p();
		xoshiro256p(u64 a, u64 b, u64 c, u64 d);
		xoshiro256p(const xoshiro256p& x);
		xoshiro256p(xoshiro256p&& x)noexcept;
		double rand01();
		double randMinMax(double min, double max);

		u64 next();
		void jump();
		void long_jump();
		static inline u64 rotl(const u64 x, int k) {
			return (x << k) | (x >> (64 - k));
		}
	private:
		std::array<u64, 4> s;
	};
}
