#include "value_io.h"
#include <string>
#include "value_parser.h"
#include <ostream>

std::ostream& operator<<(std::ostream& os, const value& v) {
	os << "value[ "
		<< std::to_string((size_t)(v.index())) << ": "
		<< value_parser::to_string(v) << " ]";
	return os;
}