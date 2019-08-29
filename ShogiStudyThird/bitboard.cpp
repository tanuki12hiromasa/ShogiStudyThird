#include "stdafx.h"
#include "bitboard.h"
#include <intrin.h>
#include <algorithm>
#include <cassert>

Bitboard::Bitboard(const std::string& bits)
	: _p({ std::stoul(bits.substr(0,9),nullptr,2),
		std::stoull(bits.substr(9, 63), nullptr, 2),
		std::stoul(bits.substr(72,9),nullptr,2) })
{}

Bitboard::Bitboard(const LittleBitboard& lb)
	: _p({ lb.side[0],lb.center,lb.side[1] })
{}

LittleBitboard Bitboard::toLittle() {
	return LittleBitboard(_p[0], _p[1], _p[2]);
}

std::string Bitboard::toString() {
	std::string str;
	for (unsigned i = 0; i < size(); i++) {
		str += test(80u - i) ? '1' : '0';
	}
	return str;
}

unsigned Bitboard::pop_first() {
	unsigned long index;
	if (_BitScanForward64(&index, _p[0])) {
		_bittestandreset64(reinterpret_cast<long long*>(&_p[0]), index);
		return index;
	}
	else if (_BitScanForward64(&index, _p[1])) {
		_bittestandreset64(reinterpret_cast<long long*>(&_p[1]), index);
		return index + 9u;
	}
	else if (_BitScanForward64(&index, _p[2])) {
		_bittestandreset64(reinterpret_cast<long long*>(&_p[2]), index);
		return index + 72u;
	}
	else {
		return size();
	}
}

unsigned Bitboard::find_first() const {
	unsigned long index;
	if (_BitScanForward64(&index, _p[0])) {
		return index;
	}
	else if (_BitScanForward64(&index, _p[1])) {
		return index + 9u;
	}
	else if (_BitScanForward64(&index, _p[2])) {
		return index + 72u;
	}
	else {
		return size();
	}
}

unsigned Bitboard::find_next(const unsigned first) const {
	//自身のfistまでを0でマスクしたBBのfind_firstを返している
	return operator&(~fillOne(first + 1u)).find_first();
}

unsigned Bitboard::find_last()const {
	unsigned long index;
	if (_BitScanReverse64(&index, _p[2])) {
		return index;
	}
	else if (_BitScanReverse64(&index, _p[1])) {
		return index;
	}
	else if (_BitScanReverse64(&index, _p[0])) {
		return index;
	}
	else {
		return size();
	}
}

unsigned Bitboard::popcount()const {
	return __popcnt64(_p[0]) + __popcnt64(_p[1]) + __popcnt64(_p[2]);
}

bool Bitboard::test(const unsigned pos)const {
	if (pos < 9u) {
		return _bittest64(reinterpret_cast<const long long*>(&_p[0]), pos) != 0;
	}
	else if (pos < 72u) {
		return _bittest64(reinterpret_cast<const long long*>(&_p[1]), pos - 9u) != 0;
	}
	else {
		return _bittest64(reinterpret_cast<const long long*>(&_p[2]), pos - 72u) != 0;
	}
}

void Bitboard::set(const unsigned pos, const bool value) {
	if (value) set(pos);
	else reset(pos);
}

void Bitboard::set(const unsigned pos) {
	assert(pos <= 80u);
	if (pos < 9u) {
		_bittestandset64(reinterpret_cast<long long*>(&_p[0]), pos);
	}
	else if (pos < 72u) {
		_bittestandset64(reinterpret_cast<long long*>(&_p[1]), pos - 9u);
	}
	else {
		_bittestandset64(reinterpret_cast<long long*>(&_p[2]), pos - 72u);
	}
}

void Bitboard::reset(const unsigned pos) {
	assert(pos <= 80u);
	if (pos < 9u) {
		_bittestandreset64(reinterpret_cast<long long*>(&_p[0]), pos);
	}
	else if (pos < 72u) {
		_bittestandreset64(reinterpret_cast<long long*>(&_p[1]), pos - 9u);
	}
	else {
		_bittestandreset64(reinterpret_cast<long long*>(&_p[2]), pos - 72u);
	}
}

void Bitboard::all_reset() {
	_p[0] = 0u; _p[1] = 0u; _p[2] = 0u;
}

bool Bitboard::none() const {
	return _p[1] == 0u && _p[0] == 0u && _p[2] == 0u;//短絡評価するので使用ビットの一番多い_p[1]をはじめに評価している
}

Bitboard Bitboard::getLineOR() const {
	Bitboard lines;
	if (_p[0] != 0) {
		lines._p[0] = 0x1FFULL;
	}
	for (unsigned i = 0; i < 7; i++) {
		if ((_p[1] & (0x1FFULL << (i * 9u))) != 0u) {
			lines._p[1] |= (0x1FFULL << (i * 9u));
		}
	}
	if (_p[2] != 0) {
		lines._p[2] = 0x1FFULL;
	}
	return lines;
}

Bitboard Bitboard::getNoFuLines()const {
	Bitboard lines(bbmask::AllOne);
	if (_p[0] != 0) {
		lines._p[0] = 0x0ULL;
	}
	for (unsigned i = 0; i < 7; i++) {
		if ((_p[1] & (0x1FFULL << (i * 9u))) != 0u) {
			lines._p[1] &= ~(0x1FFULL << (i * 9u));
		}
	}
	if (_p[2] != 0) {
		lines._p[2] = 0x0ULL;
	}
	return lines;
}

Bitboard Bitboard::fillOne(unsigned index) {
	std::array<std::uint64_t, 3> p = { 0,0,0 };
	p[0] = (1ULL << std::min(index, 9u)) - 1u;
	if (index > 9u) {
		p[1] = (1ULL << std::min(index - 9u, 63u)) - 1u;
	}
	if (index > 72u) {
		p[2] = (1ULL << std::min(index - 72u, 9u)) - 1u;
	}
	return Bitboard(std::move(p));
}
