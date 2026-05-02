#include "date_comp_ops.h"


bool operator<(const date& left, const date& right) {
	return ((left.year < right.year)
		|| (left.year == right.year && left.month < right.month)
		|| (left.year == right.year && left.month == right.month && left.day < right.day));
}

bool operator>(const date& left, const date& right) {
	return (!(left < right) && (left != right));
}