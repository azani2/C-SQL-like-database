#pragma once
#ifndef TABLE_HEADER
#define TABLE_HEADER
#include <vector>
#include "table_info.h"
#include "col_info.h"
#include "value.h"

struct table {
	table_info info;
	std::vector<col_info> cols_info;
};

#endif

