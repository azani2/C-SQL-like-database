#pragma once
#include "date.h"

namespace std {
	template<>
	struct hash<date> {
		size_t operator()(const date& d)const noexcept {
			size_t h = 0;
			h ^= std::hash<int>{}(d.year) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<unsigned int>{}(d.month) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<unsigned int>{}(d.day) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};
}