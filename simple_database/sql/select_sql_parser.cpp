#include "select_sql_parser.h"
#include <stdexcept>
#include "string_utils.h"
#include "formula_parser.h"

select_sql_parser::select_sql_parser(const std::string& s) : t(s) {
	curr = t.next();
}

select_query select_sql_parser::from_line(const std::string& s) {
	select_sql_parser p(s);
	return p.parse_line();
}

select_query select_sql_parser::parse_line() {
	if (curr.type != select_token_type::SELECT_T) {
		throw std::invalid_argument("Expected SELECT [DISTINCT] with names of columns to select but instead was: " + curr.s);
	}
	select_query res;
	res.distinct = distinct();

	std::vector<std::pair<std::string, std::string>> cols_to_select;
	get_colnames(cols_to_select, res.distinct);
	res.table_cols = cols_to_select;

	curr = t.next();
	if (curr.type != select_token_type::FROM) {
		throw std::invalid_argument("Expected FROM with a column name instead of: " + curr.s);
	}
	res.table_name = get_tb_name();

	curr = t.next(); // can be (join,) where or orderby
	switch (curr.type) {
		case select_token_type::END: {
			return res;
		}
		case select_token_type::WHERE: {
			res.has_filter = true;
			res.filter = formula_parser::formula_from_string(curr.s);
			break;
		}
		case select_token_type::ORDERBY: {
			res.sort = true;
			std::vector<std::pair<std::string, std::string>> sort_cols;
			get_colnames(sort_cols);
			res.sort_cols = sort_cols;
			return res;
			break;
		}
		default:
			throw std::invalid_argument("Unexpected trailing text in select query: " + curr.s);
	}

	curr = t.next();
	if (curr.type == select_token_type::END) {
		return res;
	}
	if (curr.type != select_token_type::ORDERBY) {
		throw std::invalid_argument("Unexpected trailing text in select query: " + curr.s);
	}

	res.sort = true;
	std::vector<std::pair<std::string, std::string>> sort_cols;
	get_colnames(sort_cols);
	res.sort_cols = sort_cols;
	return res;
}

bool select_sql_parser::distinct() const {
	size_t i = 0;
	while (i < curr.s.length() && (curr.s[i] == ' ' || curr.s[i] == '\t')) i++;
	if (i == curr.s.length()) {
		throw std::invalid_argument("Rows to select cannot be empty!");
	}
	if (i <= curr.s.length() - 8 && curr.s.substr(i, 8) == "DISTINCT") {
		if (i <= curr.s.length() - 9 && curr.s[i + 8] != ' ' && curr.s[i + 8] != '\t') {
			throw std::invalid_argument("Please separate DISTINCT and column names with a whitespace.");
		}
		return true;
	}
	return false;
}

void select_sql_parser::add_col(std::string& buf, std::vector<std::pair<std::string, std::string>>& v) const {
	string_utils::trim(buf);
	if (buf.empty()) {
		throw std::invalid_argument("Column name cannot be empty.");
	}
	std::pair<std::string, std::string> p;
	size_t dot_idx = buf.find('.');
	if (dot_idx == std::string::npos || dot_idx == 0 || dot_idx == buf.length() - 1) {
		p.first = "";
		p.second = buf;
	}
	else {
		p.first = buf.substr(0, dot_idx);
		p.second = buf.substr(dot_idx + 1);
	}
	v.push_back(p);
}

void select_sql_parser::get_colnames(std::vector<std::pair<std::string, std::string>>& v, bool dist) const {
	size_t idx_start = 0;
	if (dist) {
		idx_start = curr.s.find("DISTINCT") + 8;
	}
	size_t len = curr.s.length();
	std::string buf = "";
	for (size_t i = idx_start; i < len; i++) {
		if (curr.s[i] == ',') {
			add_col(buf, v);
			buf.clear();
		}
		else {
			buf += curr.s[i];
		}
	}
	add_col(buf, v);
}

std::string select_sql_parser::get_tb_name() const {
	std::string from_s = curr.s;
	string_utils::trim(from_s);
	if (from_s.empty()) {
		throw std::invalid_argument("Table name cannot be empty.");
	}
	if (from_s.find(' ') != std::string::npos || from_s.find('\t') != std::string::npos) {
		throw std::invalid_argument("Unexpected whitespace in table name: " + from_s);
	}
	return from_s;
}

