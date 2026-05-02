#pragma once
#ifndef TABLE_INFO_HEADER
#define TABLE_INFO_HEADER
#include <string>
#include <iostream>

struct table_info {
	std::string table_name;
	size_t col_count;
	size_t row_count;
	bool is_indexed;
	size_t index_column_index;
	size_t max_index;
};

bool operator==(const table_info&, const table_info&);
std::ostream& operator<<(std::ostream&, const table_info&);

#endif
