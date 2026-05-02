#include "col_info_io.h"
#include "value_parser.h"
#include <stdexcept>

std::ostream& operator<<(std::ostream& os, const col_info& coli) {
	os << "col_info[ "
		<< coli.col_name << ", "
		<< coli.type << ", "
		<< coli.has_default_value << ", "
		<< value_parser::to_string(coli.default_value, coli.type) << ", "
		<< coli.is_indexed << ", "
		<< coli.col_index << "]";
	return os;
}