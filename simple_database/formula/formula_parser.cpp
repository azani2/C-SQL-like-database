#include "formula_parser.h"
#include <stack>
#include <stdexcept>
#include "rel_parser.h"
#include "value_parser.h"

formula* formula_parser::formula_from_string(const std::string& s) {
	formula_parser p(s);
	formula* f = p.parse_chainf();
	if (p.curr.type != formula_token_type::FT_END) {
		throw std::invalid_argument("Unexpected trailing text: " + p.curr.s);
	}
	return f;
}

formula_parser::formula_parser(const std::string& s) : t(s) {
	curr = t.next();
}

void formula_parser::check_token_and_read_next(formula_token_type tok_t) {
	if (curr.type != tok_t) {
		throw std::invalid_argument("Unexpected token: " + curr.s);
	}
	curr = t.next();
}

formula* formula_parser::parse_formula() {
	if (curr.type == formula_token_type::T_NOT) {
		return parse_negf();
	}
	if (curr.type == formula_token_type::L_PARENTHESIS) {
		return parse_subf();
	}
	return parse_chainf();
}

formula* formula_parser::parse_negf() {
	check_token_and_read_next(formula_token_type::T_NOT);
	formula* child = parse_chainf();

	formula* f = new formula{};
	f->atomic = false;
	f->op = logic_op::NOT;
	f->children.push_back(child);
	return f;
}

formula* formula_parser::parse_subf() {
	check_token_and_read_next(formula_token_type::L_PARENTHESIS);
	formula* f = parse_chainf();
	check_token_and_read_next(formula_token_type::R_PARENTHESIS);
	return f;
}

formula* formula_parser::parse_chainf() {
	std::vector<formula*> children;
	if (curr.type != formula_token_type::COLNAME) {
		children.push_back(parse_formula());
	}
	else {
		children.push_back(parse_atomic());
	}

	bool is_chain = false;
	logic_op chain_op;

	while (curr.type == formula_token_type::T_AND || curr.type == formula_token_type::T_OR) {
		logic_op curr_op = (curr.type == formula_token_type::T_AND) ? logic_op::AND : logic_op::OR;
		if (!is_chain) {
			is_chain = true;
			chain_op = curr_op;
		}
		else if (chain_op != curr_op) {
			throw std::invalid_argument("Please use parentheses to separate diferent logical operators.");
		}
		check_token_and_read_next(curr.type);
		if (curr.type != formula_token_type::COLNAME) {
			children.push_back(parse_formula());
		}
		else {
			children.push_back(parse_atomic());
		}
	}

	if (children.size() == 1) {
		return children[0];
	}

	formula* f = new formula{};
	f->atomic = false;
	f->op = chain_op;
	f->children = children;
	return f;
}

formula* formula_parser::parse_atomic() {
	if (curr.type != formula_token_type::COLNAME) {
		throw std::invalid_argument("Expected column name instead of: " + curr.s);
	}

	std::string col = curr.s;
	check_token_and_read_next(formula_token_type::COLNAME);

	rel r = rel_parser::rel_from_string(curr.s);

	check_token_and_read_next(curr.type);

	std::string value_s = curr.s;
	check_token_and_read_next(formula_token_type::VALUE);

	formula* f = new formula{};
	f->atomic = true;
	f->comp = r;
	f->col_name = col;
	f->value_str = value_s;
	return f;
}