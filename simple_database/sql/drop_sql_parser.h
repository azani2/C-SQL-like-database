#pragma once
#ifndef DROP_SQL_PARSER_HEADER
#define DROP_SQL_PARSER_HEADER
#include <string>

class drop_sql_parser {
public:
	static std::string tb_name_from_line(std::string);
};

#endif // !DROP_SQL_PARSER_HEADER


