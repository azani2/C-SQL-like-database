#pragma once
#include "value_parser.h"
#include "string_utils.h"
#include <stdexcept> 

const std::vector<col_type> value_parser::TYPE_INDEX = {INT, DOUBLE, STRING, DATE, DATETIME};

bool value_parser::valid_date(int y, int m, int d) {
	if (m < 1 || m > 12 || d < 1) {
		return false;
	}
	static const int MONTH_DAYS[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int max_day = MONTH_DAYS[m - 1];
	if (m == 2 && y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) {
		max_day = 29;
	}
	return max_day >= d;
}
 
bool value_parser::valid_time(int h, int m, int s) {
	return (h >= 0 && h <= 23
		&& m >= 0 && m <= 59
		&& s >= 0 && s <= 59);
}

std::pair<std::string, int> value_parser::to_string_pair(const value& v, col_type t) {
	try {
		switch (t) {
		case INT:
			return { std::to_string(std::get<int>(v)), 0 };
		case DOUBLE:
			return { std::to_string(std::get<double>(v)), 0 };
		case STRING:
			return { std::get<std::string>(v), 0 };
		case DATE:
			return { string_utils::date_to_string(std::get<date>(v)), 0};
		case DATETIME:
			return { string_utils::datetime_to_string(std::get<datetime>(v)), 0 };
		default:
			throw std::invalid_argument("Unknown column type, cannot convert to string.");
		}
	}
	catch (const std::exception& e) {
		return { "", -1 };
	}
	
}

std::string value_parser::to_string(const value& v, col_type t) {
	auto def_val_res = value_parser::to_string_pair(v, t);
	if (def_val_res.second == -1) {
		throw std::logic_error("Error converting value type to string.");
	}
	return def_val_res.first;
}

std::string value_parser::to_string(const value& v) {
	col_type t = TYPE_INDEX[v.index()];
	auto def_val_res = value_parser::to_string_pair(v, t);
	if (def_val_res.second == -1) {
		throw std::logic_error("Error converting value type to string.");
	}
	return def_val_res.first;
}

int value_parser::int_from_string(const std::string& s) {
	size_t processed = 0;
	int res = std::stoi(s, &processed);
	if (processed != s.length()) {
		throw std::invalid_argument("Couldn't convert entire string " + s + " to int due to invalid characters.");
	}
	return res;
}

double value_parser::double_from_string(const std::string& s) {
	size_t processed = 0;
	double res = std::stod(s, &processed);
	if (processed != s.length()) {
		throw std::invalid_argument("Couldn't convert entire string " + s + " to double due to invalid characters.");
	}
	return res;
}

//date format is YYYY-MM-DD (lenght 10, - at indexes 4 and 7)
date value_parser::date_from_string(const std::string& s) {
	if (s.length() != 10 || s[4] != '-' || s[7] != '-') {
		throw std::invalid_argument("Invalid Date format! Expected Date format: YYYY-MM-DD");
	}
	int y = int_from_string(s.substr(0, 4));
	int m = int_from_string(s.substr(5, 2));
	int d = int_from_string(s.substr(8, 2));

	if (!valid_date(y, m, d)) {
		throw std::invalid_argument("Invalid date!");
	}
	return date{ y, (unsigned int)m, (unsigned int)d };
}

//datetime format is YYYY-MM-DD HH:MM:SS (lenght 19, : at indexes 13 and 16, ' ' at 10)
datetime value_parser::datetime_from_string(const std::string& s) {
	size_t spc_idx = s.find(' ');
	if (s.length() != 19 || s[13] != ':' || s[16] != ':' ||
		spc_idx != 10)
	{
		throw std::invalid_argument("Invalid DateTime format! Expected DateTime format: YYYY-MM-DD HH:MM:SS");
	}	

	date d = date_from_string(s.substr(0, spc_idx));

	int h = int_from_string(s.substr(11, 2));
	int m = int_from_string(s.substr(14, 2));
	int sec = int_from_string(s.substr(17, 2));
	if (!valid_time(h, m, sec)) {
		throw std::invalid_argument("Invalid time!");
	}
	return datetime{ d, (unsigned int)h, (unsigned int)m, (unsigned int)sec };
}

value value_parser::from_string(const std::string& s, col_type t) {
	switch (t) {
	case INT: {
		int res = int_from_string(s);
		return value{ res };
	}
	case DOUBLE: {
		double res = double_from_string(s);
		return value{ res };
	}
	case STRING: {
		return value{ s };
	}
	case DATE: {
		date res = date_from_string(s);
		return value{ res };
	}
	case DATETIME: {
		datetime res = datetime_from_string(s);
		return value{ res };
	}
	default:
		throw std::invalid_argument("Unknown column type, cannot convert to value type.");
	}
}

