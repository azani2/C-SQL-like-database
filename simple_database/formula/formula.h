#pragma once
#ifndef FORMULA_HEADER
#define FORMULA_HEADER
#include <string>
#include <vector>
#include "rel.h"
#include "value.h"
#include "logic_op.h"

struct formula {
	bool atomic;

	rel comp;
	std::string col_name;
	size_t col_idx;
	std::string value_str;
	value v;

	logic_op op;
	std::vector<formula*> children;

	formula() = default;

	formula(const formula&) = delete;
	formula& operator=(const formula&) = delete;

	~formula() {
		for (formula* child : children) {
			delete child;
		}
	}
};

#endif // !FORMULA_HEADER


