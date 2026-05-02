#pragma once
#ifndef FORMULA_EVALUATOR_HEADER
#define FORMULA_EVALUATOR_HEADER
#include "formula.h"
#include "row.h"

class formula_evaluator {
public:
	static bool eval(const formula*, const row&);
};

#endif // !FORMULA_EVALUATOR_HEADER


