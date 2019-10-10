#include "stdafx.h"
#include "bb_kiki.h"
#include "koma_motion.h"
#include "vec2.h"
#include <intrin.h>



void BBkiki::init() {
	genData();
}

const Bitboard BBkiki::getSKyouDashKiki(const Bitboard& allbb, const unsigned komapos) {
	//利きテーブルをindexから参照して利きを求める 利きテーブルの結果は1筋(side[0])にあるので他の筋の場合はシフトさせる
	if (komapos < 9u) {//1筋はy=komaposなので%9しなくていい
		auto p = _pext_u64(allbb._p[0], sKyouMask[komapos]);
		return Bitboard(sKyouDash[sKIndex[komapos] + p], 0u, 0u);
	}
	else if (komapos < 72u) {//2-8筋
		unsigned x = komapos / 9 - 1;//_p[1]内でのxなので将棋盤全体のxより1小さい
		auto p = _pext_u64(allbb._p[1], sKyouMask[komapos % 9] << (x * 9));//対象の筋までmaskを移動
		return Bitboard(0u, sKyouDash[sKIndex[komapos % 9] + p] << (x * 9u), 0u);//結果を対象の筋にシフトしBBを返す
	}
	else {//9筋
		auto p = _pext_u64(allbb._p[2], sKyouMask[komapos % 9]);
		return Bitboard(0u, 0u, sKyouDash[sKIndex[komapos % 9] + p]);
	}
}
const Bitboard BBkiki::getGKyouDashKiki(const Bitboard& allbb, const unsigned komapos) {
	if (komapos < 9u) {//1筋
		auto p = _pext_u64(allbb._p[0], gKyouMask[komapos]);
		return Bitboard(gKyouDash[gKIndex[komapos] + p], 0u, 0u);
	}
	else if (komapos < 72u) {//2-8筋
		unsigned x = komapos / 9 - 1;
		auto p = _pext_u64(allbb._p[1], gKyouMask[komapos % 9] << (x * 9));//対象の筋までmaskを移動
		return Bitboard(0u, gKyouDash[gKIndex[komapos % 9] + p] << (x * 9u), 0u);//結果を対象の筋にシフトしBBを返す
	}
	else {//9筋
		auto p = _pext_u64(allbb._p[2], gKyouMask[komapos % 9]);
		return Bitboard(0u, 0u, gKyouDash[gKIndex[komapos % 9] + p]);
	}
}
const Bitboard BBkiki::getHiDashKiki(const Bitboard& allbb, const unsigned komapos) {
	Bitboard kiki;
	//縦
	if (komapos < 9u) {//1筋
		auto p = _pext_u64(allbb._p[0], HiVerticalMask[komapos]);
		kiki |= Bitboard(HiDashVertical[HIndex[komapos] + p], 0u, 0u);
	}
	else if (komapos < 72u) {//2-8筋
		unsigned x = komapos / 9 - 1;
		auto p = _pext_u64(allbb._p[1], HiVerticalMask[komapos % 9] << (x * 9));//対象の筋までmaskを移動
		kiki |= Bitboard(0u, HiDashVertical[HIndex[komapos % 9] + p] << (x * 9u), 0u);//結果を対象の筋にシフトしBBを返す
	}
	else {//9筋
		auto p = _pext_u64(allbb._p[2], HiVerticalMask[komapos % 9]);
		kiki |= Bitboard(0u, 0u, HiDashVertical[HIndex[komapos % 9] + p]);
	}
	//横
	{
		unsigned x = komapos / 9u, y = komapos % 9u;
		auto p = _pext_u64(allbb._p[1], HiHorizontalMask[komapos / 9] << y);//対象の段までmaskを移動させてpext
		auto bb = HiDashHorizontal[HIndex[x] + p];
		kiki |= Bitboard(static_cast<std::uint64_t>(bb.side[0]) << y, bb.center << y, static_cast<std::uint64_t>(bb.side[1]) << y);
	}
	return kiki;
}
const Bitboard BBkiki::getKakuDashKiki(const Bitboard& allbb, const unsigned komapos) {
	Bitboard kiki;
	//posi
	{
		auto p = _pext_u64(allbb._p[1], KakuPositiveInclinationMask[komapos]);
		kiki |= Bitboard(KakuDashPositive[KPIndex[komapos] + p]);
	}
	//nega
	{
		auto p = _pext_u64(allbb._p[1], KakuNegativeInclinationMask[komapos]);
		kiki |= Bitboard(KakuDashNegative[KNIndex[komapos] + p]);
	}
	return kiki;
}

void BBkiki::genData() {
	genStepTable();
	genMask();
	genIndex();
	genDashTable();
}

#pragma optimize("",off)//最適化すると何故か実行時エラーを吐くのでこの関数のみ最適化を無効化している
void BBkiki::genMask() {
	//端の情報は利きの判定には不要なので、bitboardの真ん中部分の中7段分のみを利用すればよいので、maskはuint64で作る
	//香mask
	{	//先手香
		for (unsigned long long y = 0; y < 9; y++) {
			std::uint64_t u = 0;
			for (unsigned long long t = 1; t < y; t++) {
				assert(t >= 0);
				_bittestandset64(reinterpret_cast<long long*>(&u), t);
			}
			sKyouMask[y] = u;
		}
		//後手香
		for (unsigned long long y = 0; y < 9; y++) {
			std::uint64_t u = 0;
			for (unsigned long long t = 7; t > y; t--) {
				assert(t >= 0);
				_bittestandset64(reinterpret_cast<long long*>(&u), t);
			}
			gKyouMask[y] = u;
		}
	}
	//飛車mask
	{	//縦 
		for (unsigned long long y = 0; y < 9; y++) {
			std::uint64_t u = 0b011111110ULL;
			_bittestandreset64(reinterpret_cast<long long*>(&u), y);
			HiVerticalMask[y] = u;
		}
		//横
		for (unsigned long long x = 0; x < 9; x++) {
			std::uint64_t u = 0b000000001000000001000000001000000001000000001000000001000000001ULL;
			if (1 <= x && x <= 7) {
				_bittestandreset64(reinterpret_cast<long long*>(&u), (x - 1ull) * 9ull);
			}
			HiHorizontalMask[x] = u;
		}
	}
	//角mask
	{	//傾き正
		for (unsigned long long i = 0; i < 81; i++) {
			Bitboard u;
			for (long long x = (i / 9) - 1, y = (i % 9) - 1; 1 <= x && x <= 7 && 1 <= y && y <= 7; x--, y--) {
				u.set(x * 9ull + y);
			}
			for (long long x = (i / 9) + 1, y = (i % 9) + 1; 1 <= x && x <= 7 && 1 <= y && y <= 7; x++, y++) {
				u.set(x * 9ull + y);
			}
			KakuPositiveInclinationMask[i] = u._p[1];
		}
		//傾き負
		for (unsigned long long i = 0; i < 81; i++) {
			Bitboard u;
			for (long long x = (i / 9) + 1, y = (i % 9) - 1; 1 <= x && x <= 7 && 1 <= y && y <= 7; x++, y--) {
				u.set(x * 9 + y);
			}
			for (long long x = (i / 9) - 1, y = (i % 9) + 1; 1 <= x && x <= 7 && 1 <= y && y <= 7; x--, y++) {
				u.set(x * 9 + y);
			}
			KakuNegativeInclinationMask[i] = u._p[1];
		}
	}
}

#pragma optimize("",on)
void BBkiki::genIndex() {
	__popcnt64(0xFFULL);
	unsigned sum = 0, count = 0;
	for (unsigned long i = 0; i < 81; i++) {
		KPIndex[i] = sum;
		count = __popcnt64(KakuPositiveInclinationMask[i]);
		sum += 1 << count;
	}
	sum = 0, count = 0;
	for (unsigned long i = 0; i < 81; i++) {
		KNIndex[i] = sum;
		count = __popcnt64(KakuNegativeInclinationMask[i]);
		sum += 1 << count;
	}
	sum = 0, count = 0;
	for (unsigned long i = 0; i < 9; i++) {
		HIndex[i] = sum;
		count = __popcnt64(HiVerticalMask[i]);
		sum += 1 << count;
	}
	sum = 0, count = 0;
	for (unsigned long i = 0; i < 9; i++) {
		sKIndex[i] = sum;
		count = __popcnt64(sKyouMask[i]);
		sum += 1 << count;
	}
	sum = 0, count = 0;
	for (unsigned long i = 0; i < 9; i++) {
		gKIndex[i] = sum;
		count = __popcnt64(gKyouMask[i]);
		sum += 1 << count;
	}
}

inline Bitboard tobikiki(koma::Vector2 from, koma::Vector2 step, Bitboard allBB) {
	Bitboard bb;
	for (koma::Vector2 v = from + step; v.isInside(); v += step) {
		bb.set(v.tou());
		if (allBB.test(v.tou())) break;
	}
	return bb;
}

void BBkiki::genDashTable() {
	using namespace koma;
	//_pdep_u64()
	//sKyou
	for (unsigned y = 0; y < 9; y++) {
		std::uint64_t count = 0x1ULL << __popcnt64(sKyouMask[y]);//盤状態の組み合わせは2^(maskの立っているbit数)個になる
		for (std::uint64_t k = 0; k < count; k++) {
			Bitboard ban(_pdep_u64(k, sKyouMask[y]), 0u, 0u);//pextの逆演算で盤面を再現する 縦の利きは1筋で計算する
			Bitboard kiki = tobikiki(Vector2(0, y), motion::Up, ban);//再現した盤面から利きを計算する
			sKyouDash[sKIndex[y] + k] = kiki._p[0];
		}
	}
	//gKyou
	for (unsigned y = 0; y < 9; y++) {
		std::uint64_t count = 1ULL << __popcnt64(gKyouMask[y]);
		for (std::uint64_t k = 0; k < count; k++) {
			Bitboard ban(_pdep_u64(k, gKyouMask[y]), 0u, 0u);
			Bitboard kiki = tobikiki(Vector2(0, y), motion::Down, ban);
			gKyouDash[gKIndex[y] + k] = kiki._p[0];
		}
	}
	//HiVirtical
	for (unsigned y = 0; y < 9; y++) {
		std::uint64_t count = 1ULL << __popcnt64(HiVerticalMask[y]);
		for (std::uint64_t k = 0; k < count; k++) {
			Bitboard ban(_pdep_u64(k, HiVerticalMask[y]), 0u, 0u);
			Bitboard kiki = tobikiki(Vector2(0, y), motion::Up, ban) | tobikiki(Vector2(0, y), motion::Down, ban);
			HiDashVertical[HIndex[y] + k] = kiki._p[0];
		}
	}
	//HiHorizontal
	for (unsigned x = 0; x < 9; x++) {
		std::uint64_t count = 1ULL << __popcnt64(HiHorizontalMask[x]);
		for (std::uint64_t k = 0; k < count; k++) {
			Bitboard ban(0u, _pdep_u64(k, HiHorizontalMask[x]), 0u); //横の利きは1段で計算する
			Bitboard kiki = tobikiki(Vector2(x, 0), motion::Right, ban) | tobikiki(Vector2(x, 0), motion::Left, ban);
			HiDashHorizontal[HIndex[x] + k] = kiki.toLittle();
		}
	}
	//KakuPosi
	for (unsigned pos = 0; pos < 81; pos++) {
		unsigned x = pos / 9, y = pos % 9;
		std::uint64_t count = 1ULL << __popcnt64(KakuPositiveInclinationMask[pos]);
		for (std::uint64_t k = 0; k < count; k++) {
			Bitboard ban(0u, _pdep_u64(k, KakuPositiveInclinationMask[pos]), 0u);
			Bitboard kiki = tobikiki(Vector2(x, y), motion::UpRight, ban) | tobikiki(Vector2(x, y), motion::DownLeft, ban);
			KakuDashPositive[KPIndex[pos] + k] = kiki.toLittle();
		}
	}
	//KakuNega
	for (unsigned pos = 0; pos < 81; pos++) {
		unsigned x = pos / 9, y = pos % 9;
		std::uint64_t count = 1ULL << __popcnt64(KakuNegativeInclinationMask[pos]);
		for (std::uint64_t k = 0; k < count; k++) {
			Bitboard ban(0u, _pdep_u64(k, KakuNegativeInclinationMask[pos]), 0u);
			Bitboard kiki = tobikiki(Vector2(x, y), motion::UpLeft, ban) | tobikiki(Vector2(x, y), motion::DownRight, ban);
			KakuDashNegative[KNIndex[pos] + k] = kiki.toLittle();
		}
	}
}

inline void ippokiki(BBkiki::LBarray81& lbs, const std::vector<koma::Vector2>& steps) {
	using namespace koma;
	for (unsigned pos = 0; pos < 81; pos++) {
		Bitboard kiki;
		Vector2 from(pos);
		for (Vector2 step : steps) {
			auto v = from + step;
			if (v.isInside()) {
				kiki.set(v.tou());
			}
		}
		lbs[pos] = kiki.toLittle();
	}
}

void BBkiki::genStepTable() {
	using namespace koma;
	ippokiki(sFu, { motion::Up });
	ippokiki(sKei, { motion::UpUpLeft,motion::UpUpRight });
	ippokiki(sGin, { motion::Up,motion::UpLeft,motion::UpRight,motion::DownLeft,motion::DownRight });
	ippokiki(sKin, { motion::Up,motion::UpLeft,motion::UpRight,motion::Left,motion::Right,motion::Down });
	ippokiki(gFu, { motion::Down });
	ippokiki(gKei, { motion::DownDownLeft,motion::DownDownRight });
	ippokiki(gGin, { motion::Down,motion::UpLeft,motion::UpRight,motion::DownLeft,motion::DownRight });
	ippokiki(gKin, { motion::Up,motion::DownLeft,motion::DownRight,motion::Left,motion::Right,motion::Down });
	ippokiki(UmaStep, { motion::Up,motion::Left,motion::Right,motion::Down });
	ippokiki(RyuStep, { motion::UpLeft,motion::UpRight,motion::DownLeft,motion::DownRight });
	ippokiki(Ou, { motion::Up,motion::Left,motion::Right,motion::Down, motion::UpLeft,motion::UpRight,motion::DownLeft,motion::DownRight });
}
