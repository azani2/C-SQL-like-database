#pragma once
#ifndef FORMULA_PARSER_HEADER
#define FORMULA_PARSER_HEADER
#include "formula.h"
#include "formula_tokenizer.h"
#include <string>

class formula_parser {
private:
	formula_tokenizer t;
	formula_token  curr;
	
	formula_parser(const std::string&);
	void check_token_and_read_next(formula_token_type);
	formula* parse_formula();
	formula* parse_negf();
	formula* parse_subf();
	formula* parse_chainf();
	formula* parse_atomic();

public:
	static formula* formula_from_string(const std::string&);
};

#endif // !FORMULA_PARSER_HEADER