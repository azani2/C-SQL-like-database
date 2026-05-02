#include "catch_amalgamated.hpp"
#include "select_sql_parser.h"
#include "test_utils.h"

TEST_CASE("parse_line parses simple query") {
	std::string s = " * FROM Sample";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("", "*");
	select_query expected("Sample",table_cols,0,0,nullptr,0,sort_cols );
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses simple query with ORDER BY") {
	std::string s = " * FROM Sample ORDER BY Name";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("", "*");
	sort_cols.emplace_back("", "Name");
	select_query expected("Sample", table_cols, 0, 0, nullptr, 1, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses simple query with DISTINCT") {
	std::string s = " DISTINCT * FROM Sample";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("", "*");
	select_query expected("Sample", table_cols, 1, 0, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses simple query with DISTINCT and WHERE") {
	std::string s = " DISTINCT * FROM Sample WHERE Sample.Name > \"P\"";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("", "*");
	select_query expected("Sample", table_cols, 1, 1, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses simple query with colnames") {
	std::string s = " Sample.Name, Age, Sample.Grade  FROM Sample";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("Sample", "Name");
	table_cols.emplace_back("", "Age");
	table_cols.emplace_back("Sample", "Grade");
	select_query expected("Sample", table_cols, 0, 0, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses simple query with colnames and distinct") {
	std::string s = "DISTINCT Sample.Name, Age, Sample.Grade  FROM Sample";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("Sample", "Name");
	table_cols.emplace_back("", "Age");
	table_cols.emplace_back("Sample", "Grade");
	select_query expected("Sample", table_cols, 1, 0, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses simple query with colnames and where") {
	std::string s = " Sample.Name, Age, Sample.Grade  FROM Sample WHERE Age > \"20\"";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("Sample", "Name");
	table_cols.emplace_back("", "Age");
	table_cols.emplace_back("Sample", "Grade");
	select_query expected("Sample", table_cols, 0, 1, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses simple query with distinct, colnames and where") {
	std::string s = "DISTINCT  Sample.Name, Age, Sample.Grade  FROM Sample WHERE Age > \"20\"";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("Sample", "Name");
	table_cols.emplace_back("", "Age");
	table_cols.emplace_back("Sample", "Grade");
	select_query expected("Sample", table_cols, 1, 1, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses query with AND in WHERE") {
	std::string s =" Sample.Name, Age FROM Sample WHERE Age > \"18\" AND Name == \"Peni\"";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("Sample", "Name");
	table_cols.emplace_back("", "Age");

	select_query expected("Sample", table_cols, 0, 1, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses query with OR in WHERE") {
	std::string s = " * FROM Sample WHERE Age < \"10\" OR Age > \"65\"";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("", "*");

	select_query expected("Sample", table_cols, 0, 1, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses query with parentheses in WHERE") {
	std::string s = " * FROM Sample WHERE (Age > \"18\" AND Age < \"30\")";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("", "*");

	select_query expected("Sample", table_cols, 0, 1, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses query with NOT and parentheses") {
	std::string s =" * FROM Sample WHERE NOT (Age < \"18\")";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("", "*");

	select_query expected("Sample", table_cols, 0, 1, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses DISTINCT query with complex WHERE") {
	std::string s = "DISTINCT Sample.Name, Age FROM Sample WHERE  (Age > \"20\" AND Age < \"60\") OR Name == \"Maria\"";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("Sample", "Name");
	table_cols.emplace_back("", "Age");

	select_query expected("Sample", table_cols, 1, 1, nullptr, 0, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses query with ORDER BY only") {
	std::string s = " * FROM Sample ORDER BY Age";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("", "*");
	sort_cols.emplace_back("", "Age");

	select_query expected("Sample", table_cols, 0, 0, nullptr, 1, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses query with WHERE and ORDER BY") {
	std::string s = " Sample.Name, Age FROM Sample WHERE Age > \"18\" ORDER BY Name";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("Sample", "Name");
	table_cols.emplace_back("", "Age");
	sort_cols.emplace_back("", "Name");

	select_query expected("Sample", table_cols, 0, 1, nullptr, 1, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line parses query with all tokens types") {
	std::string s = "DISTINCT Sample.Name, Age, Sample.Grade FROM Sample WHERE NOT (Age < \"18\" OR Name == \"Test\") ORDER BY Grade";
	std::vector<std::pair<std::string, std::string>> table_cols;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	table_cols.emplace_back("Sample", "Name");
	table_cols.emplace_back("", "Age");
	table_cols.emplace_back("Sample", "Grade");

	sort_cols.emplace_back("", "Grade");

	select_query expected("Sample", table_cols, 1, 1, nullptr, 1, sort_cols);
	compare_select_queries(expected, select_sql_parser::from_line(s));
}

TEST_CASE("parse_line throws when select columns are empty") {
	std::string s = " FROM Sample";
	REQUIRE_THROWS_WITH(select_sql_parser::from_line(s), "Expected SELECT [DISTINCT] with names of columns to select but instead was: Sample");
}

TEST_CASE("parse_line throws when query starts with WHERE") {
	std::string s = "WHERE Age > \"10\"";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Expected SELECT [DISTINCT] with names of columns to select but instead was: Age > \"10\""
	);
}

TEST_CASE("parse_line throws when FROM is missing") {
	std::string s = "* ";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Expected FROM with a column name instead of: "
	);
}

TEST_CASE("parse_line throws when WHERE is before FROM") {
	std::string s = "* WHERE Age > \"10\"";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Expected FROM with a column name instead of: "
	);
}

TEST_CASE("parse_line throws when there is text after FROM table name") {
	std::string s = "* FROM Sample abc";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Unexpected trailing text in select query: abc"
	);
	s = "Sample.Name, Age FROM Sample abc";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Unexpected trailing text in select query: abc"
	);
}

TEST_CASE("parse_line throws when there is trailing text after WHERE") {
	std::string s = "* FROM Sample WHERE Age > \"10\" abc";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Unexpected trailing text: abc"
	);
}

TEST_CASE("parse_line throws when ORDER keyword is wrong") {
	std::string s = "* FROM Sample WHERE Age > \"10\" ORDER Age";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Unexpected trailing text: ORDER"
	);
}

TEST_CASE("parse_line throws when FROM is not separated from table name") {
	std::string s = "* FROMSample WHERE Age > \"10\" ORDER BY Age";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Please separate FROM and the table name with a whitespace."
	);
}

TEST_CASE("parse_line throws when ORDER BY is not separated from table name") {
	std::string s = "* FROM Sample WHERE Age > \"10\" ORDER BYAge";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Please separate ORDER BY clause and column names with a whitespace!"
	);
}

TEST_CASE("parse_line throws when DISTINCT is not separated from column name") {
	std::string s = "DISTINCT* FROM Sample WHERE Age > \"10\" ORDER BY Age";
	REQUIRE_THROWS_WITH(
		select_sql_parser::from_line(s),
		"Please separate DISTINCT and column names with a whitespace."
	);
}