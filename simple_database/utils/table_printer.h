#pragma once
#ifndef TABLE_PRINTER_HEADER
#define TABLE_PRINTER_HEADER
#include "col_info.h"
#include <vector>
#include <string>
#include "row.h"

class table_printer {
private:
	std::vector<size_t> max_col_lengths;

	size_t table_width() const;
	void update_max_col_lengths(const std::vector<std::string>&);

public:
	table_printer(const std::vector<size_t>&);
	void print_header(const std::vector<std::string>&);
	void print_rows(const std::vector<row>&) const;
};

#endif // !TABLE_PRINTER_HEADER



