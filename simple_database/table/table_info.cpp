#include "table_info.h"

bool operator==(const table_info& tbi1, const table_info& tbi2) {
	return (tbi1.table_name == tbi2.table_name
		&& tbi1.col_count == tbi2.col_count
		&& tbi1.row_count == tbi2.row_count
		&& tbi1.is_indexed == tbi2.is_indexed
		&& tbi1.index_column_index == tbi2.index_column_index
		&& tbi1.max_index == tbi2.max_index);
}

std::ostream& operator<<(std::ostream& os, const table_info& tbi) {
	os << "table_info["
		<< tbi.table_name << ", "
		<< tbi.col_count << ", "
		<< tbi.row_count << ", "
		<< tbi.is_indexed << ", "
		<< tbi.index_column_index << ", "
		<< tbi.max_index << "]";
	return os;
}