#include "random.h"
#include <limits>
#include <random>

namespace Random {
	xoshiro256p::xoshiro256p() {
		std::random_device rd;
		std::uniform_int_distribution urandom(std::numeric_limits<u64>::min(), std::numeric_limits<u64>::max());
		s[0] = urandom(rd);
		s[1] = urandom(rd);
		s[2] = urandom(rd);
		s[3] = urandom(rd);
	}
	xoshiro256p::xoshiro256p(u64 a, u64 b, u64 c, u64 d)
		:s({ a,b,c,d })
	{}
	xoshiro256p::xoshiro256p(const xoshiro256p& x)
		:s(x.s)
	{}
	xoshiro256p::xoshiro256p(xoshiro256p&& x) noexcept
		:s(std::move(x.s))
	{}

	double xoshiro256p::rand01() {
		return static_cast<double>(next()) / std::numeric_limits<u64>::max();
	}
	double xoshiro256p::randMinMax(double min, double max) {
		return min + rand01() * (max - min);
	}

	u64 xoshiro256p::next() {
		const u64 result = s[0] + s[3];

		const u64 t = s[1] << 17;

		s[2] ^= s[0];
		s[3] ^= s[1];
		s[1] ^= s[2];
		s[0] ^= s[3];

		s[2] ^= t;

		s[3] = rotl(s[3], 45);

		return result;
	}

	void xoshiro256p::jump() {
		static constexpr u64 JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

		u64 s0 = 0;
		u64 s1 = 0;
		u64 s2 = 0;
		u64 s3 = 0;
		for (int i = 0; i < sizeof JUMP / sizeof * JUMP; i++)
			for (int b = 0; b < 64; b++) {
				if (JUMP[i] & UINT64_C(1) << b) {
					s0 ^= s[0];
					s1 ^= s[1];
					s2 ^= s[2];
					s3 ^= s[3];
				}
				next();
			}

		s[0] = s0;
		s[1] = s1;
		s[2] = s2;
		s[3] = s3;
	}

	void xoshiro256p::long_jump() {
		static constexpr u64 LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

		u64 s0 = 0;
		u64 s1 = 0;
		u64 s2 = 0;
		u64 s3 = 0;
		for (int i = 0; i < sizeof LONG_JUMP / sizeof * LONG_JUMP; i++)
			for (int b = 0; b < 64; b++) {
				if (LONG_JUMP[i] & UINT64_C(1) << b) {
					s0 ^= s[0];
					s1 ^= s[1];
					s2 ^= s[2];
					s3 ^= s[3];
				}
				next();
			}

		s[0] = s0;
		s[1] = s1;
		s[2] = s2;
		s[3] = s3;
	}

}