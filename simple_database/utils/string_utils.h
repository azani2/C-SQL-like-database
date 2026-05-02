#pragma once
#ifndef STRING_UTILS_HEADER
#define STRING_UTILS_HEADER
#include <string>
#include "date.h"
#include "datetime.h"

class string_utils {
private:
	static std::string size_t_to_string_4_digit(const size_t&);
	static std::string size_t_to_string_2_digit(const size_t&);
public:
	static void remove_trailing_spaces(std::string&);
	static void trim(std::string&);
	static void unquote(std::string&);
	static std::string date_to_string(const date&);
	static std::string datetime_to_string(const datetime&);
};

#endif 
