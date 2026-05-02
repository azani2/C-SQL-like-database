#pragma once
#ifndef VALUE_PARSER_HEADER
#define VALUE_PARSER_HEADER
#include "col_type.h"
#include "value.h"

class value_parser {
private:
	static const std::vector<col_type> TYPE_INDEX;

	static int int_from_string(const std::string&);
	static double double_from_string(const std::string&);
	static date date_from_string(const std::string&);
	static datetime datetime_from_string(const std::string&);
	static std::pair<std::string, int> to_string_pair(const value&, col_type);

public:
	static bool valid_date(int, int, int);
	static bool valid_time(int, int, int);
	static value from_string(const std::string&, col_type);
	static std::string to_string(const value&);
	static std::string to_string(const value&, col_type);
};

#endif 

