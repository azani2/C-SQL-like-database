#pragma once
#ifndef COL_INFO_HEADER
#define COL_INFO_HEADER
#include <string>
#include <iostream>
#include "col_type.h"
#include "value.h"

struct col_info {
	std::string col_name;
	col_type type;

	bool has_default_value;
	value default_value;

	bool is_indexed;
	size_t col_index;
};

bool operator==(const col_info&, const col_info&);

#endif