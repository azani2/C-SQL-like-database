#include "table_printer.h"
#include "value_parser.h"

table_printer::table_printer(const std::vector<size_t>& column_max_sizes) : max_col_lengths(column_max_sizes) {}

size_t table_printer::table_width() const {
	size_t width = 3;
	for (size_t col_w : max_col_lengths) {
		width += 3;
		width += col_w;
	}
	return width;
}

void table_printer::update_max_col_lengths(const std::vector<std::string>& fields) {
	size_t size = fields.size();
	if (size != max_col_lengths.size()) {
		throw std::invalid_argument("Invalid column count " + std::to_string(size) + ", expected: " + std::to_string(max_col_lengths.size()));
	}
	for (size_t i = 0; i < size; i++) {
		if (fields[i].length() > max_col_lengths[i]) {
			max_col_lengths[i] = fields[i].length();
		}
	}
}

void table_printer::print_header(const std::vector<std::string>& colnames) {
	update_max_col_lengths(colnames);
	size_t tbw = table_width();
	std::cout << std::string(tbw, '-') << std::endl;
	std::string s = " | ";
	size_t size = colnames.size();
	for (size_t i = 0; i < size; i++) {
		s.append(colnames[i]);
		size_t padding_length = max_col_lengths[i] - colnames[i].length();
		s.append(std::string(padding_length, ' '));
		s.append(" | ");
	}
	std::cout << s << std::endl;
	std::cout << std::string(tbw, '-') << std::endl;
}

void table_printer::print_rows(const std::vector<row>& rows) const {
	for (auto& r : rows) {
		std::string s = " | ";
		size_t size = r.size();
		for (size_t i = 0; i < size; i++) {
			std::string val_s = value_parser::to_string(r[i]);
			s.append(val_s);
			size_t padding_length = max_col_lengths[i] - val_s.length();
			s.append(std::string(padding_length, ' '));
			s.append(" | ");
		}
		std::cout << s << std::endl;
	}
}