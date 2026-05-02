#pragma once
#ifndef ROW_COMPARER_HEADER
#define ROW_COMPARER_HEADER
#include "row.h"
#include <vector>

class row_comparer {
public:
	static bool compare_rows(const row&, const row&, const std::vector<size_t>&);
	static bool eq_rows(const row&, const row&, const std::vector<size_t>&);
};

#endif // !ROW_COMPARER_HEADER


