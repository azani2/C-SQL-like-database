#pragma once
#ifndef CSV_PARSER_HEADER
#define CSV_PARSER_HEADER
#include "table_info.h"
#include <string>
#include <sstream>
#include <vector>
#include "col_info.h"

class csv_parser {
	static std::string string_to_csv_format(const std::string&);
public:
	static std::string tb_name_from_csv(const std::string&);

	static table_info tb_info_from_csv(const std::string&);
	static std::string tb_info_to_csv(const table_info&);

	static col_info col_info_from_csv(const std::string&);
	static std::string col_info_to_csv(const col_info&);

	static row row_from_csv(const std::string&, const std::vector<col_info>&);
	static std::string row_to_csv(const row&);

	template<typename T>
	static std::string vector_to_csv(const std::vector<T>& v) {
		std::ostringstream ss("");
		size_t size = v.size();
		for (size_t i = 0; i < size; i++) {
			ss << v[i];
			if (i < size - 1) {
				ss << ',';
			}
		}
		return ss.str();
	}
};

#endif


