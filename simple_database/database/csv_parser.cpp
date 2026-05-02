#include "csv_parser.h"
#include "column_types.h"
#include "value_parser.h"
#include <sstream>

// TODO: check if input strings are in the right format and add exception handling

std::string csv_parser::tb_name_from_csv(const std::string& s) {
	size_t idx = s.find(',');
	return s.substr(0, idx);
}

table_info csv_parser::tb_info_from_csv(const std::string& s) {
	table_info res;

	size_t commas_count = 0;
	std::string buf = "";
	for (size_t i = 0; i < s.length(); i++) {
		if (s[i] != ',') {
			buf += s[i];
			continue;
		}

		commas_count++;
		std::stringstream ss(buf);
		switch (commas_count) {
		case 1:
			res.table_name = buf;
			break;
		case 2:
			ss >> res.col_count;
			break;
		case 3:
			ss >> res.row_count;
			break;
		case 4:
			ss >> res.is_indexed;
			break;
		case 5:
			ss >> res.index_column_index;
			break;
		default:
			break;
		}
		buf.clear();
	}
	std::stringstream ss(buf);
	ss >> res.max_index;
	return res;
}

std::string csv_parser::tb_info_to_csv(const table_info& tbi) {
	std::string res = "";
	res.append(tbi.table_name);
	res.append(",");
	res.append(std::to_string(tbi.col_count));
	res.append(",");
	res.append(std::to_string(tbi.row_count));
	res.append(",");
	res.append(std::to_string((int)tbi.is_indexed));
	res.append(",");
	res.append(std::to_string(tbi.index_column_index));
	res.append(",");
	res.append(std::to_string(tbi.max_index));
	return res;
}


col_info csv_parser::col_info_from_csv(const std::string& s) {
	col_info res;

	size_t commas_count = 0;
	std::string buf = "";
	for (size_t i = 0; i < s.length(); i++) {
		if (s[i] != ',') {
			buf += s[i];
			continue;
		}
		else if (i > 0 && s[i - 1] == '\\') {
			buf.erase(buf.length() - 1, 1);
			buf += s[i];
			continue;
		}

		commas_count++;
		std::stringstream ss(buf);
		switch (commas_count) {
		case 1:
			res.col_name = buf;
			break;
		case 2:
			res.type = column_types::col_type_from_string(buf);
			break;
		case 3:
			ss >> res.has_default_value;
			break;
		case 4:
			res.default_value = value_parser::from_string(buf, res.type);
			break;
		case 5:
			ss >> res.is_indexed;
			break;
		default:
			break;
		}
		buf.clear();
	}
	std::stringstream ss(buf);
	ss >> res.col_index;
	return res;
}

std::string csv_parser::col_info_to_csv(const col_info& coli) {
	std::string res = "";
	res.append(coli.col_name);
	res.append(",");
	res.append(column_types::col_type_to_string(coli.type));
	res.append(",");
	res.append(std::to_string((int)coli.has_default_value));
	res.append(",");
	res.append(string_to_csv_format(value_parser::to_string(coli.default_value, coli.type)));
	res.append(",");
	res.append(std::to_string((int)coli.is_indexed));
	res.append(",");
	res.append(std::to_string(coli.col_index));
	return res;
}

std::string csv_parser::string_to_csv_format(const std::string& s) {
	std::string formatted;
	size_t l = s.length();
	for (size_t i = 0; i < l; i++) {
		if (s[i] == ',') {
			formatted.append("\\,");
		}
		else {
			formatted += s[i];
		}
	}
	return formatted;
}

row csv_parser::row_from_csv(const std::string& line, const std::vector<col_info>& cols) {
	std::string buf = "";
	row res;
	size_t col_n = 0;
	size_t cols_size = cols.size();
	col_info curr_col = cols[0];
	for (size_t i = 0; i < line.length(); i++) {
		if (line[i] == ',' && (i < 1 || line[i - 1] != '\\')) {
			col_n++;
			if (col_n >= cols_size) {
				throw std::invalid_argument("There are too many fields in this csv record:\n"
					+ line
					+ "\nFound: " + std::to_string(col_n + 1) + ", expected: " + std::to_string(cols_size) + " fields.");
			}
			value v = value_parser::from_string(buf, curr_col.type);
			res.push_back(v);

			curr_col = cols[col_n];
			buf.clear();
		}
		else if (i > 0 && line[i] == ',' && line[i - 1] == '\\') {
			buf.pop_back();
			buf += line[i];
		}
		else {
			buf += line[i];
		}
	}

	if (col_n != cols_size - 1) {
		throw std::invalid_argument("There are not enough fields in this csv record:\n"
			+ line
			+ "\nFound: " + std::to_string(col_n + 1) + ", expected: " + std::to_string(cols_size) + " fields.");
	}

	value v = value_parser::from_string(buf, curr_col.type);
	res.push_back(v);
	return res;
}

std::string csv_parser::row_to_csv(const row& r) {
	std::string res = "";
	size_t size = r.size();
	size_t last = size - 1;
	for (size_t i = 0; i < size; i++) {
		res.append(string_to_csv_format(value_parser::to_string(r[i])));
		if (i != last) {
			res.append(",");
		}
	}
	return res;
}