#pragma once
#ifndef SELECT_SQL_PARSER_HEADER
#define SELECT_SQL_PARSER_HEADER
#include "select_query.h"
#include "select_tokenizer.h"

class select_sql_parser {
private:
	select_tokenizer t;
	select_token curr;

	select_sql_parser(const std::string&);
	select_query parse_line();

	bool distinct() const;
	void get_colnames(std::vector<std::pair<std::string, std::string>>&, const bool dist = false) const;
	void add_col(std::string&, std::vector<std::pair<std::string, std::string>>&) const;
	std::string get_tb_name() const;

public:
	static select_query from_line(const std::string&);
};

#endif


