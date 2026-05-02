#pragma once
#ifndef COLUMN_TYPES_HEADER
#define COLUMN_TYPES_HEADER
#include <string>
#include "col_type.h"
#include "value.h"

class column_types {
public:
	static value db_default_value(const col_type&);
	static bool is_valid(const std::string&);
	static std::string col_type_to_string(const col_type&);
	static col_type col_type_from_string(const std::string&);
};

#endif
