#include "datetime.h"

bool operator==(const datetime& dt1, const datetime& dt2) {
	return (dt1.dt == dt2.dt
		&& dt1.hr == dt2.hr
		&& dt1.min == dt2.min
		&& dt1.sec == dt2.sec);
}