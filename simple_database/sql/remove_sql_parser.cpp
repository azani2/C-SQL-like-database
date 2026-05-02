#include "remove_sql_parser.h"
#include "formula_parser.h"
#include <stdexcept>

std::string remove_sql_parser::data(const std::string& line) {
	size_t data_start = line.find("FROM ");
	if (data_start == std::string::npos) {
		throw std::invalid_argument("After \"Remove\" expected FROM statement followed by a table name!");
	}
	data_start += 5;
	std::string trimmed = line.substr(data_start);
	string_utils::trim(trimmed);
	return trimmed;
}

std::string remove_sql_parser::table_name_from_line(const std::string& line) {
	std::string info = data(line);
	size_t separator_idx = info.find(' ');
	if (separator_idx == std::string::npos) {
		return info;
	}
	return info.substr(0, separator_idx);
}

formula* remove_sql_parser::formula_from_line(const std::string& line) {
	std::string info = data(line);
	string_utils::trim(info);
	size_t separator_idx = info.find(' ');
	if (separator_idx == std::string::npos) {
		return nullptr;
	}
	std::string formula_s = info.substr(separator_idx + 1);
	string_utils::trim(formula_s);
	size_t formula_start_idx = formula_s.find("WHERE ");
	if (formula_start_idx == std::string::npos) {
		throw std::invalid_argument("Unparsable text after table name: " + formula_s);
	}
	formula_start_idx += 6;
	return formula_parser::formula_from_string(formula_s.substr(formula_start_idx));
}
