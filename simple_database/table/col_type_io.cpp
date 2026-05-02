#include "col_type_io.h"
#include "column_types.h"

std::ostream& operator<<(std::ostream& os, const col_type& t) {
	os << column_types::col_type_to_string(t);
	return os;
}