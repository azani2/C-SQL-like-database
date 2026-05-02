#pragma once
#ifndef REMOVE_SQL_PARSER_HEADER
#define REMOVE_SQL_PARSER_HEADER
#include <string>
#include "formula.h"
#include "string_utils.h"

class remove_sql_parser {
private:
	static std::string data(const std::string&);
public:
	static std::string table_name_from_line(const std::string&);
	static formula* formula_from_line(const std::string&);
};

#endif // !REMOVE_SQL_PARSER_HEADER


