#pragma once
#ifndef SELECT_QUERY_HEADER
#define SELECT_QUERY_HEADER
#include <vector>
#include <string>
#include "formula.h"

struct select_query {
	std::string table_name;
	std::vector<std::pair<std::string, std::string>> table_cols;

	bool distinct;

	bool has_filter;
	formula* filter;

	bool sort;
	std::vector<std::pair<std::string, std::string>> sort_cols;

	select_query();
	select_query(const std::string&,
		std::vector<std::pair<std::string, std::string>>&,
		bool,
		bool,
		formula*,
		bool,
		std::vector<std::pair<std::string, std::string>>);
};

#endif // !SELECT_QUERY_HEADER

