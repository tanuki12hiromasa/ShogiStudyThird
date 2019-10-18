#pragma once
#include <cassert>

namespace koma {
	class Vector2 {
	public:
		int x;
		int y;

		Vector2() :x(0), y(0){}
		Vector2(int x, int y) : x(x), y(y) {}
		Vector2(int pos) : x(pos / 9), y(pos % 9) {}

		inline bool isDivisible(const Vector2& v);
		inline int toi();
		inline unsigned tou();
		inline operator int();
		inline bool isInside();

		inline bool operator==(const Vector2& v);
		inline bool operator!=(const Vector2& v);
		inline Vector2& operator+=(const Vector2& v);
		inline Vector2& operator-=(const Vector2& v);
		inline Vector2& operator*=(const int c);
		inline Vector2 operator+(const Vector2& v)const;
		inline Vector2 operator-(const Vector2& v)const;
		inline Vector2 operator*(const int c)const;
		friend Vector2 operator*(const int c, Vector2 v);
		inline int operator/(const Vector2& v);
		inline Vector2 operator/(const int c);
		inline Vector2& operator/=(const int c);
		inline Vector2 operator-() const;
	};

	inline int Vector2::toi() {
		return x * 9 + y;
	}
	inline unsigned Vector2::tou() {
		return 9ul * x + y;
	}
	inline Vector2::operator int() {
		return toi();
	}
	inline bool Vector2::isInside() {
		return 0 <= x && x < 9 && 0 <= y && y < 9;
	}

	inline bool Vector2::operator==(const Vector2& v) {
		return x == v.x && y == v.y;
	}
	inline bool Vector2::operator!=(const Vector2& v) {
		return !operator==(v);
	}
	inline Vector2& Vector2::operator+=(const Vector2& v) {
		x += v.x; y += v.y;
		return *this;
	}
	inline Vector2& Vector2::operator-=(const Vector2& v) {
		x -= v.x; y -= v.y;
		return *this;
	}
	inline Vector2& Vector2::operator*=(const int c) {
		x *= c; y *= c;
		return *this;
	}
	inline Vector2 Vector2::operator+(const Vector2& v)const {
		return Vector2(x + v.x, y + v.y);
	}
	inline Vector2 Vector2::operator-(const Vector2& v)const {
		return Vector2(x - v.x, y - v.y);
	}

	inline Vector2 Vector2::operator*(const int c)const {
		return Vector2(c * x, c * y);
	}
	inline Vector2 operator*(const int c, Vector2 v) {
		return v *= c;
	}
	inline int Vector2::operator/(const Vector2& v) {
		assert(isDivisible(v));
		return x / v.x;
	}
	inline Vector2 Vector2::operator/(const int c) {
		return Vector2(x / c, y / c);
	}
	inline Vector2& Vector2::operator/=(const int c) {
		x /= c; y /= c;
		return *this;
	}
	inline Vector2 Vector2::operator-() const {
		return Vector2(-x, -y);
	}

}