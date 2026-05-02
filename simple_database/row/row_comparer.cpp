#include "row_comparer.h"
#include "row.h"
#include "date_comp_ops.h"
#include "datetime_comp_ops.h"

bool row_comparer::compare_rows(const row& v1, const row& v2, const std::vector<size_t>& keys) {
	for (int idx : keys) {
		if (v1[idx] < v2[idx]) {
			return true;
		}
		if (v1[idx] > v2[idx]) {
			return false;
		}
	}
	return false;
}

bool row_comparer::eq_rows(const row& v1, const row& v2, const std::vector<size_t>& keys) {
	for (int idx : keys) {
		if (v1[idx] != v2[idx]) {
			return false;
		}
	}
	return true;
}