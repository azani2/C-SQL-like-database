#include "col_info.h"

bool operator==(const col_info& coli1, const col_info& coli2) {
	return (coli1.col_name == coli2.col_name
		&& coli1.type == coli2.type
		&& coli1.has_default_value == coli2.has_default_value
		&& coli1.default_value == coli2.default_value
		&& coli1.is_indexed == coli2.is_indexed
		&& coli1.col_index == coli2.col_index);
}