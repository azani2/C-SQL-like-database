#pragma once
#ifndef CREATE_SQL_PARSER_HEADER
#define CREATE_SQL_PARSER_HEADER
#include "table_info.h"
#include "table.h"
#include "col_info.h"
#include <vector>

class create_sql_parser {
private:
	//col_info name, type and default value parsers
	static size_t schema_colon_index(const std::string& s);
	static std::string col_name_from_schema(const std::string&);
	static std::string col_type_from_schema(const std::string&);
	static bool col_has_default_from_schema(const std::string&);
	static value col_defaul_value_from_schema(const std::string&, col_type);

	//table_info name and index parsers
	static std::string table_name_from_line(const std::string&);
	static bool has_index_from_line(const std::string&);
	static std::string idx_col_name_from_line(const std::string&);

	//table vector<col_info> parser
	static col_info col_info_from_token(std::string);
	static void check_no_duplicate_columns(const std::vector<col_info>&);
	static void cols_info_from_schema(const std::string&, std::vector<col_info>&);

	static std::string extract_schema(const std::string&);
public:
	static table tb_from_line(const std::string&);
};

#endif
