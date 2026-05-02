#include "value_comparer.h"
#include "datetime_comp_ops.h"
#include "date_comp_ops.h"
#include <stdexcept>

bool value_comparer::compare_values(const value& left, const value& right, const rel& rel) {
	if (left.index() != right.index()) {
		throw std::invalid_argument("Values to compare are of different types.");
	}
	switch (rel)
	{
	case LESS:
		return left < right;
	case BIGGER:
		return left > right;
	case EQUAL:
		return left == right;
	default:
		throw std::invalid_argument("Unknown comparison operator.");
		break;
	}
}