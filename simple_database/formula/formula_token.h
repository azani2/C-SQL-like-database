#pragma once
#ifndef FORMULA_TOKEN_HEADER
#define FORMULA_TOKEN_HEADER
#include <string>

enum formula_token_type {
	COLNAME, VALUE,
	T_LESS, T_BIGGER, T_EQUAL,
	T_AND, T_OR, T_NOT,
	L_PARENTHESIS, R_PARENTHESIS,
	FT_END
};

struct formula_token {
	formula_token_type type;
	std::string s;
};

#endif // !TOKEN_HEADER

