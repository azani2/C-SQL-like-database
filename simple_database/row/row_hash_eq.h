#pragma once
#ifndef ROW_HASH_EQ_HEADER
#define ROW_HASH_EQ_HEADER
#include "row.h"
#include "date_hash.h"
#include "datetime_hash.h"
#include <functional>
#include <vector>
#include <variant>

struct row_hash {
	size_t operator()(const row& r) const noexcept {
		size_t h = 0;
		for (const auto& x : r) {
			size_t value_hash = std::visit([](const auto& v) -> size_t {
				using T = std::decay_t<decltype(v)>;
				return std::hash<T>{}(v);
				}, x);

			h ^= value_hash + 0x9e3779b9 + (h << 6) + (h >> 2);
		}
		return h;
	}
};

struct row_eq {
	bool operator()(const row& r1, const row& r2) const noexcept {
		return r1 == r2;
	}
};

#endif // !ROW_HASH_EQ_HEADER

