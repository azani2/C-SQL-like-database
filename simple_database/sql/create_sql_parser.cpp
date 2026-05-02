#pragma once
#include "create_sql_parser.h"
#include "column_types.h"
#include <stdexcept>
#include "value_parser.h"
#include "string_utils.h"
#include <unordered_set>

std::string create_sql_parser::table_name_from_line(const std::string& s) {
	size_t spc = s.find(" (");
	if (spc == std::string::npos) {
		throw std::invalid_argument("Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
	}
	std::string name = s.substr(0, spc);
	string_utils::trim(name);
	if (name.empty()) {
		throw std::invalid_argument("Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
	}
	return name;
}

size_t create_sql_parser::schema_colon_index(const std::string& s) {
	size_t colon_index = s.find(':');
	if (colon_index == 0 || colon_index == std::string::npos || colon_index == s.length() - 1) {
		throw std::invalid_argument("Column description is not in the right format!");
	}
	return colon_index;
}

std::string create_sql_parser::col_name_from_schema(const std::string& s) {
	size_t colon_index = schema_colon_index(s);
	std::string name = s.substr(0, colon_index);
	return name;
}

std::string create_sql_parser::col_type_from_schema(const std::string& s) {
	size_t type_start = schema_colon_index(s) + 1;
	size_t type_length = s.length();
	size_t spc_idx = s.find(' ');
	if (spc_idx != std::string::npos) {
		type_length = spc_idx - type_start;
	}
	std::string type = s.substr(type_start, type_length);
	if (type.empty()) {
		throw std::invalid_argument("Column description is not in the right format!");
	}
	return type;
}

bool create_sql_parser::col_has_default_from_schema(const std::string& s) {
	if (s.find(" DEFAULT") == std::string::npos) {
		return false;
	}
	return true;
}

value create_sql_parser::col_defaul_value_from_schema(const std::string& s, col_type t) {
	size_t idx = s.find(" DEFAULT ");
	if (idx == std::string::npos || s.length() < idx + 10) {
		throw std::invalid_argument("Default value string is empty!");
	}
	idx += 9;
	std::string value_string = s.substr(idx);
	string_utils::remove_trailing_spaces(value_string);
	value def_val = value_parser::from_string(value_string, t);
	return def_val;
}

col_info create_sql_parser::col_info_from_token(std::string token) {
	col_info coli;
	string_utils::remove_trailing_spaces(token);
	coli.col_name = col_name_from_schema(token);
	coli.type = column_types::col_type_from_string(col_type_from_schema(token));

	coli.has_default_value = col_has_default_from_schema(token);
	if (coli.has_default_value) {
		coli.default_value = col_defaul_value_from_schema(token, coli.type);
	}
	else {
		coli.default_value = column_types::db_default_value(coli.type);
	}
	return coli;
}

void create_sql_parser::check_no_duplicate_columns(const std::vector<col_info>& cols_info) {
	std::unordered_set<std::string> column_names;
	for (col_info coli : cols_info) {
		if (column_names.contains(coli.col_name)) {
			throw std::invalid_argument("Table cannot have 2 columns with the same name: " + coli.col_name);
		}
		column_names.insert(coli.col_name);
	}
}

void create_sql_parser::cols_info_from_schema(const std::string& s, std::vector<col_info>& cols_info) {
	std::string piece = "";
	size_t col_idx = 0;
	for (size_t i = 0; i < s.length(); i++) {
		// If the ',' is escaped with "\'", then read it as part of the piece
		if (s[i] == ',' &&  (i == 0 || s[i - 1] != '\\') ) {
			col_info coli = col_info_from_token(piece);
			coli.is_indexed = false;
			coli.col_index = col_idx;
			cols_info.push_back(coli);		
			piece.clear();
			col_idx++;
		}
		else if (i > 0 && s[i - 1] == '\\' && s[i] == ',') {
			piece.erase(piece.length() - 1, 1);
			piece += s[i];
		}
		else {
			piece += s[i];
		}
	}
	string_utils::trim(piece);
	col_info coli = col_info_from_token(piece);
	coli.is_indexed = false;
	coli.col_index = col_idx;
	cols_info.push_back(coli);
	check_no_duplicate_columns(cols_info);
}

//receives only trimmed text after schema
//checks if create query has index clause
bool create_sql_parser::has_index_from_line(const std::string& s) {
	if (s.find("Index ON") != std::string::npos) {
		return true;
	}
	if (!s.empty()) {
		throw std::invalid_argument("Unexpected trailing text after columns description: " + s);
	}
	return false;
}

//receives only trimmed text after schema
//is only called if create query has index clause
std::string create_sql_parser::idx_col_name_from_line(const std::string& s) {
	size_t idx_col_name_start_idx = s.find("Index ON ");
	if (idx_col_name_start_idx == std::string::npos || s.length() < idx_col_name_start_idx + 10) {
		throw std::invalid_argument("No index column was specified after Index ON clause.");
	}
	idx_col_name_start_idx += 9;
	return s.substr(idx_col_name_start_idx);
}

std::string create_sql_parser::extract_schema(const std::string& s) {
	size_t schema_start = s.find(" (") + 1;
	size_t schema_end = s.find_last_of(')');
	if (schema_start == std::string::npos || schema_end == std::string::npos || schema_end < schema_start) {
		throw std::invalid_argument("Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
	}
	std::string schema = s.substr(schema_start + 1, schema_end - schema_start - 1);
	string_utils::trim(schema);
	return schema;
}

//read table schema -> create a vector of col_info
//user schema should look like (ColumnName:ColumnType, ...) [Index ON SomeColumnName]
table create_sql_parser::tb_from_line(const std::string& s) {
	table_info basic_info;
	basic_info.table_name = table_name_from_line(s);
	basic_info.row_count = 0;
	basic_info.max_index = 0;

	std::string schema = extract_schema(s);
	std::vector<col_info> cols_info;
	cols_info_from_schema(schema, cols_info);
	basic_info.col_count = cols_info.size();

	std::string index_info = "";
	size_t index_info_start = s.find_last_of(')') + 1;

	if (index_info_start >= s.length()) {
		basic_info.is_indexed = false;
	}
	else {
		index_info = s.substr(index_info_start);
		string_utils::trim(index_info);
		basic_info.is_indexed = has_index_from_line(index_info);
	}

	if (basic_info.is_indexed) {
		bool found_idx_col = false;
		std::string idx_col_name = idx_col_name_from_line(index_info);
		for (col_info& coli : cols_info) {
			if (coli.col_name == idx_col_name) {
				if (coli.type != INT) {
					throw std::invalid_argument("Index column must be of type Int!");
				}
				if (coli.has_default_value) {
					throw std::invalid_argument("Index column cannot have default value!");
				}
				coli.is_indexed = true;
				basic_info.index_column_index = coli.col_index;
				found_idx_col = true;
				break;
			}
		}
		if (!found_idx_col) {
			throw std::invalid_argument("Couldn't match index column name " + idx_col_name + " to any column name!");
		}
	}
	else {
		basic_info.index_column_index = 0;
	}

	return table(basic_info, cols_info);
}
