#include "datetime_comp_ops.h"
#include "date_comp_ops.h"

bool operator<(const datetime& left, const datetime& right) {
	return ((left.dt < right.dt)
		|| (left.dt == right.dt && left.hr < right.hr)
		|| (left.dt == right.dt && left.hr == right.hr && left.min < right.min)
		|| (left.dt == right.dt && left.hr == right.hr && left.min == right.min && left.sec < right.sec));
}

bool operator>(const datetime& left, const datetime& right) {
	return (!(left < right) && (left != right));
}