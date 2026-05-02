#include "column_types.h"
#include <string>
#include <unordered_set>
#include <stdexcept>
#include "value_parser.h"

static const std::vector<std::pair<col_type, std::string>> VALID_TYPES =
{
	{ INT, "Int" },
	{ STRING, "String" },
	{ DOUBLE, "Double" },
	{ DATE, "Date" },
	{ DATETIME, "DateTime" }
};

static const std::vector<std::pair<col_type, std::string>> DEFAULT_VALUES =
{
	{ INT, "0" },
	{ STRING, "" },
	{ DOUBLE, "0" },
	{ DATE, "1000-01-01" },
	{ DATETIME, "1000-01-01 00:00:00" }
};

value column_types::db_default_value(const col_type& t) {
	for (auto& p : DEFAULT_VALUES) {
		if (t == p.first) {
			return value_parser::from_string(p.second, t);
		}
	}
	throw std::invalid_argument("Unknown column type.");
}

bool column_types::is_valid(const std::string& s) {
	for (auto& p : VALID_TYPES) {
		if (p.second == s) {
			return true;
		}
	}
	return false;
}

std::string column_types::col_type_to_string(const col_type& t) {
	for (auto& p : VALID_TYPES) {
		if (p.first == t) {
			return p.second;
		}
	}
	throw std::invalid_argument("Unknown column type.");
}

col_type column_types::col_type_from_string(const std::string& s) {
	for (auto& p : VALID_TYPES) {
		if (p.second == s) {
			return p.first;
		}
	}
	throw std::invalid_argument("Unknown column type name.");
}