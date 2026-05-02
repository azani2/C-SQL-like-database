#include "test_utils.h"
#include "catch_amalgamated.hpp"
#include <random>
#include "string_utils.h"
#include "select_query.h"

void compare_row(const row& expected, const row& actual) {
	REQUIRE(expected.size() == actual.size());
	size_t size = expected.size();
	for (size_t i = 0; i < size; i++) {
		REQUIRE(expected[i] == actual[i]);
	}
}

void compare_rows(const std::vector<row>& expected, const std::vector<row>& actual) {
	REQUIRE(expected.size() == actual.size());
	size_t sz = expected.size();
	for (size_t i = 0; i < sz; i++) {
		compare_row(expected[i], actual[i]);
	}
}

void generate_test_data(std::string& s, const table& t, const size_t& ROWS, std::vector<row>& expected) {
	s = "INTO ";
	s.append(t.info.table_name);
	s.append(" {");

	std::mt19937 rng(12345);
	for (size_t i = 1; i < ROWS; i++) {
		if (i > 0) {
			s.append(", ");
		}
		s.append("(");
		row curr;

		int id = 1000 + i;
		curr.push_back(id);

		if (rng() ^ 1) {
			s.append("A Name");
			curr.push_back("A Name");
		}
		else {
			curr.push_back(t.cols_info[1].default_value);
		}
		s.append(",");

		double score = (double)(rng() % 1000) / 67.67;
		std::string score_s = std::to_string(score);
		s.append(score_s);
		curr.push_back(std::stod(score_s));
		s.append(",");

		int year = 2000 + (i % 5);
		unsigned int month = (i % 12) + 1;
		unsigned int day = (i % 28) + 1;
		date birthdate = date{ year, month, day };
		s.append(string_utils::date_to_string(birthdate));
		s.append(", ");
		curr.push_back(birthdate);

		year = 2018 + (i % 10);
		unsigned int hr = i % 24;
		unsigned int min = i % 60;
		unsigned int sec = min;
		datetime dt = datetime{ date{year, month, day}, hr, min, sec };
		s.append(string_utils::datetime_to_string(dt));
		s.append(")");
		curr.push_back(dt);
		expected.push_back(curr);
	}
	s.append("}");
}

table simple_table() {
	table t;
	t.info = table_info{ "Sample", 3, 0, false, 0, 0 };
	t.cols_info = {
		col_info{"A", INT, false, 0, false, 0},
		col_info{"B", INT, false, 0, false, 1},
		col_info{"C", INT, false, 0, false, 2}
	};
	return t;
}

template<typename T>
void eq_vector(const std::vector<T>& v1, const std::vector<T>& v2) {
	REQUIRE(v1.size() == v2.size());
	size_t size = v1.size();
	for (size_t i = 0; i < size; i++) {
		REQUIRE(v1[i] == v2[i]);
	}
}

void compare_select_queries(select_query s1, select_query s2) {
	REQUIRE(s1.table_name == s2.table_name);
	REQUIRE(s1.distinct == s2.distinct);
	REQUIRE(s1.has_filter == s2.has_filter);
	REQUIRE(s1.sort == s2.sort);
	eq_vector(s1.table_cols, s2.table_cols);
	eq_vector(s1.sort_cols, s2.sort_cols);
}

void compare_tbi_to(const table_info& tbi, std::string tb_name,
	size_t col_count,
	size_t row_count,
	bool indexed,
	size_t ind_col,
	size_t max_idx) {

	REQUIRE(tbi.table_name == tb_name);
	REQUIRE(tbi.col_count == col_count);
	REQUIRE(tbi.row_count == row_count);
	REQUIRE(tbi.is_indexed == indexed);
	REQUIRE(tbi.index_column_index == ind_col);
	REQUIRE(tbi.max_index == max_idx);
}