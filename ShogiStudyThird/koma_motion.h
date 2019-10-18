#pragma once
#include "vec2.h"
#include <vector>

namespace koma {
	namespace motion {
		const Vector2 Up(0, -1);
		const Vector2 Down(0, 1);
		const Vector2 Right(-1, 0);
		const Vector2 Left(1, 0);
		const Vector2 UpRight(-1, -1);
		const Vector2 UpLeft(1, -1);
		const Vector2 DownRight(-1, 1);
		const Vector2 DownLeft(1, 1);
		const Vector2 UpUpRight(-1, -2);
		const Vector2 UpUpLeft(1, -2);
		const Vector2 DownDownRight(-1, 2);
		const Vector2 DownDownLeft(1, 2);

	}
}