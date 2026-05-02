#include "insert_sql_parser.h"
#include "string_utils.h"
#include <stdexcept>
#include "value_parser.h"

const std::string insert_sql_parser::INVALID_MSG = "Invalid insert query";
const std::string insert_sql_parser::MISSING_NAME_OR_ROWS_MSG = ": Table name cannot be empty and must be followed by \" {rows to be inserted}\".";
const std::string insert_sql_parser::INVALID_ROWS = ": Rows must be described as \"(value, ..., value)\".";

std::string insert_sql_parser::tb_name_from_line(const std::string& line) {
	size_t idx = line.find("INTO ");
	if (idx == std::string::npos) {
		throw std::invalid_argument(INVALID_MSG + ": \"Insert INTO \" must be followed by a valid table name.");
	}
	size_t idx_start_name = idx + 5;
	idx = line.substr(idx_start_name).find(" {");
	if (idx == std::string::npos) {
		throw std::invalid_argument(INVALID_MSG + MISSING_NAME_OR_ROWS_MSG);
	}
	size_t idx_end = idx + idx_start_name;
	if (idx_start_name >= idx_end) { //Name is empty
		throw std::invalid_argument(INVALID_MSG + MISSING_NAME_OR_ROWS_MSG);
	}

	std::string tb_name = line.substr(idx_start_name, idx_end - idx_start_name);
	if (tb_name.find(' ') != std::string::npos) {
		throw std::invalid_argument(INVALID_MSG + ": Table name cannot contain spaces!");
	}
	return tb_name;
}

// INTO Sample {(1, "Test", 1), (2, "something else", 100)}
void insert_sql_parser::rows_from_line(const std::string& line, const table& t, std::vector<row>& v) {
	v.clear();
	get_rows(line, v, t);
}

void insert_sql_parser::get_rows(const std::string& line, std::vector<row>& v, const table& t) {
	size_t length = line.length();
	row curr;
	size_t row_number = 1;
	size_t i = 0;
	i = line.find(" {");
	if (i == std::string::npos) {
		throw std::invalid_argument(INVALID_MSG + MISSING_NAME_OR_ROWS_MSG);
	}
	size_t closing_bracket_idx = line.find_last_of('}');
	if (closing_bracket_idx == std::string::npos) {
		throw std::invalid_argument(INVALID_MSG + MISSING_NAME_OR_ROWS_MSG);
	}
	if (closing_bracket_idx == i + 2) {
		throw std::invalid_argument(INVALID_MSG + ": Rows schema cannot be empty.");
	}
	for (; i < length; i++) {
		if (i > closing_bracket_idx && line[i] != ' ') {
			throw std::invalid_argument(INVALID_MSG + ": There is unparsable text after end of rows.");
		}
		if (line[i] == '(') {
			// Res is index of next ) or 0
			int res = extract_next_row(line.substr(i), t, curr, row_number++);
			if (res < 1) {
				throw std::invalid_argument(INVALID_MSG + INVALID_ROWS);
			}
			v.push_back(curr);
			curr.clear();
			i += res + 1;
		}
	}
}

// s is like (.........
int insert_sql_parser::extract_next_row(const std::string& s, const table& t, row& r, const size_t& row_number) {
	size_t len = s.length();
	bool found_closing_bracket = false;

	size_t col_idx = 0;
	int idx_col_idx = (t.info.is_indexed ? t.info.index_column_index : -1);

	std::string buf;
	size_t i;
	for (i = 1; i < len; i++) {
		if (i > 0 && s[i - 1] == '\\' && (s[i] == ',' || s[i] == ')')) {
			buf.erase(buf.length() - 1, 1);
			buf += s[i];
		}
		else if ((s[i] == ',' || s[i] == ')')) {
			if (col_idx == idx_col_idx) {
				int idx = t.info.max_index + row_number;
				r.push_back(idx);
				col_idx++;
			}
			string_utils::remove_trailing_spaces(buf);
			value v;
			if (buf.empty() && t.cols_info[col_idx].has_default_value) {
				v = t.cols_info[col_idx].default_value;
			}
			else {
				v = value_parser::from_string(buf, t.cols_info[col_idx].type);
			}
			r.push_back(v);
			if (s[i] == ')') {
				found_closing_bracket = true;
				break;
			}
			buf.clear();
			col_idx++;
		}
		else {
			buf += s[i];
		}
	}

	if (!found_closing_bracket) {
		throw std::invalid_argument(INVALID_MSG + INVALID_ROWS);
	}

	while (r.size() < t.info.col_count
		&& (t.cols_info[r.size()].is_indexed || t.cols_info[r.size()].has_default_value))
	{
		if (t.cols_info[r.size()].is_indexed) {
			int idx = t.info.max_index + row_number;
			r.push_back(idx);
		}
		else if (t.cols_info[r.size()].has_default_value) {
			r.push_back(t.cols_info[r.size()].default_value);
		}
	}

	if (r.size() != t.info.col_count) {
		throw std::invalid_argument("Row " + s.substr(0, i + 1) + " has invalid number of columns: " + std::to_string(r.size()) + ", expected: " + std::to_string(t.info.col_count) + ".");
	}
	return i;
}

void insert_sql_parser::rows_from_line_faster(const std::string& line, const table& t, std::vector<row>& v) {
	v.clear();
	size_t rows_start_idx = line.find(" {");
	if (rows_start_idx == std::string::npos) {
		throw std::invalid_argument(INVALID_MSG + MISSING_NAME_OR_ROWS_MSG);
	}
	rows_start_idx++;
	get_rows_faster(line, rows_start_idx, v, t);
}

void insert_sql_parser::get_rows_faster(const std::string& line, const size_t& rows_start_idx, std::vector<row>& v, const table& t) {
	size_t length = line.length();
	const char* data = line.c_str();
	row curr;
	size_t row_number = 1;
	size_t i = 0;
	i = rows_start_idx;

	size_t closing_bracket_idx = line.find_last_of('}');
	if (closing_bracket_idx == std::string::npos) {
		throw std::invalid_argument(INVALID_MSG + MISSING_NAME_OR_ROWS_MSG);
	}
	if (closing_bracket_idx == i + 1) {
		throw std::invalid_argument(INVALID_MSG + ": Rows schema cannot be empty.");
	}
	for (; i < length; i++) {
		if (i > closing_bracket_idx && data[i] != ' ' && data[i] != '\t') {
			throw std::invalid_argument(INVALID_MSG + ": There is unparsable text after end of rows.");
		}
		if (data[i] == '(') {
			// Res is index of next ) or 0
			int res = extract_next_row_faster(data + i, t, curr, row_number++);
			if (!res) {
				throw std::invalid_argument(INVALID_MSG + INVALID_ROWS);
			}
			v.push_back(curr);
			curr.clear();
			i += res;
		}
	}
}

int insert_sql_parser::extract_next_row_faster(const char* s, const table& t, row& r, const size_t& row_number) {
	bool found_closing_bracket = false;
	size_t col_idx = 0;
	int idx_col_idx = (t.info.is_indexed ? t.info.index_column_index : -1);
	std::string buf;
	size_t i = 1;
	for (;; i++) {
		char c = s[i];
		if (c == '\0') break;
		if (i > 0 && s[i - 1] == '\\' && (c == ',' || c == ')')) {
			buf.pop_back();
			buf += c;
		}
		else if (c == ',' || c == ')') {
			if (col_idx == idx_col_idx) {
				int idx = t.info.max_index + row_number;
				r.push_back(idx);
				col_idx++;
			}
			string_utils::remove_trailing_spaces(buf);
			value v;
			if (buf.empty() && t.cols_info[col_idx].has_default_value) {
				v = t.cols_info[col_idx].default_value;
			}
			else {
				v = value_parser::from_string(buf, t.cols_info[col_idx].type);
			}
			r.push_back(v);
			if (c == ')') {
				found_closing_bracket = true;
				break;
			}
			buf.clear();
			col_idx++;
		}
		else {
			buf += c;
		}
	}

	if (!found_closing_bracket) {
		throw std::invalid_argument(INVALID_MSG + INVALID_ROWS);
	}

	while (r.size() < t.info.col_count
		&& (t.cols_info[r.size()].is_indexed || t.cols_info[r.size()].has_default_value))
	{
		if (t.cols_info[r.size()].is_indexed) {
			int idx = t.info.max_index + row_number;
			r.push_back(idx);
		}
		else if (t.cols_info[r.size()].has_default_value) {
			r.push_back(t.cols_info[r.size()].default_value);
		}
	}

	if (r.size() != t.info.col_count) {
		throw std::invalid_argument("Row #" + std::to_string(row_number) + " has invalid number of columns: " + std::to_string(r.size()) + ", expected: " + std::to_string(t.info.col_count) + ".");
	}
	return (int)i;
}