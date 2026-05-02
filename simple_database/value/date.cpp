#include "date.h"

bool operator==(const date& d1, const date& d2) {
	return (d1.year == d2.year 
		&& d1.month == d2.month
		&& d1.day == d2.day);
}