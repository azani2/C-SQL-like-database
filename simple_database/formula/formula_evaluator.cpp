#include "formula_evaluator.h"
#include "value_comparer.h"
#include <stdexcept>

bool formula_evaluator::eval(const formula* f, const row& r) {
	if (!f) {
		return true;
	}
	if (f->atomic) {
		if (f->col_idx >= r.size()) {
			throw std::invalid_argument("Invalid index " + std::to_string(f->col_idx) + " for row with "
				+ std::to_string(r.size()) + " columns.");
		}
		return value_comparer::compare_values(r[f->col_idx], f->v, f->comp);
	}
	if (f->op == logic_op::OR) {
		for (formula* child : f->children) {
			if (eval(child, r)) {
				return true;
			}
		}
		return false;
	}
	if (f->op == logic_op::NOT) {
		if (f->children.size() < 1) {
			throw std::invalid_argument("There is no subformula to evaluate with NOT.");
		}
		if (eval(f->children[0], r)) {
			return false;
		}
		return true;
	}
	bool res = true;
	for (formula* child : f->children) {
		res = res && (eval(child, r));
		if (!res) {
			return false;
		}
	}
	return true;
}