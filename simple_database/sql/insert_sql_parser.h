#pragma once
#ifndef INSERT_SQL_PARSER_HEADER
#define INSERT_SQL_PARSER_HEADER
#include "row.h"
#include "table.h"

class insert_sql_parser {
private:
	static const std::string INVALID_MSG;
	static const std::string MISSING_NAME_OR_ROWS_MSG;
	static const std::string INVALID_ROWS;

	static int extract_next_row(const std::string&, const table&, row&, const size_t&);
	static void get_rows(const std::string&, std::vector<row>&, const table&);
	
	static int extract_next_row_faster(const char*, const table&, row&, const size_t&);
	static void get_rows_faster(const std::string&, const size_t&, std::vector<row>&, const table&);

public:
	//Passed string must be trimmed!
	static std::string tb_name_from_line(const std::string&);
	static void rows_from_line(const std::string&, const table&, std::vector<row>&);
	
	static void rows_from_line_faster(const std::string&, const table&, std::vector<row>&);
};

#endif // !INSERT_SQL_PARSER_HEADER


