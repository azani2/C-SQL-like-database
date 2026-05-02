#pragma once
#ifndef VALUE_COMPARER_HEADER
#define VALUE_COMPARER_HEADER
#include "rel.h"
#include "value.h"

class value_comparer {
private:
public:
	static bool compare_values(const value&, const value&, const rel&);
};

#endif // !VALUE_COMPARER_HEADER



