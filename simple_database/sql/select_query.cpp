#include "select_query.h"

select_query::select_query()
	:
	table_name(""),
	distinct(false),
	has_filter(false),
	filter(nullptr),
	sort(false) {};

select_query::select_query(const std::string& s,
	std::vector<std::pair<std::string, std::string>>& tb_cols,
	bool dist,
	bool filtered,
	formula* filter,
	bool sorted,
	std::vector<std::pair<std::string, std::string>> sort_cols)
	:
	table_name(s),
	table_cols(tb_cols),
	distinct(dist),
	has_filter(filtered),
	filter(filter),
	sort(sorted),
	sort_cols(sort_cols) {};