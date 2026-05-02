#pragma once
#ifndef TEST_UTILS_HEADER
#define TEST_UTILS_HEADER
#include "row.h"
#include "table.h"
#include "select_query.h"

void compare_row(const row& expected, const row& actual);
void compare_rows(const std::vector<row>& expected, const std::vector<row>& actual);

void generate_test_data(std::string& s, const table& t, const size_t& ROWS, std::vector<row>& expected);
table simple_table();

template<typename T>
void eq_vector(const std::vector<T>& v1, const std::vector<T>& v2);

void compare_select_queries(select_query, select_query);

void compare_tbi_to(const table_info& tbi, std::string tb_name,
	size_t col_count,
	size_t row_count,
	bool indexed,
	size_t ind_col,
	size_t max_idx);

#endif // !TEST_UTILS_HEADER
