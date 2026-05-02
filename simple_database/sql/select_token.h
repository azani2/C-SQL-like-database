#pragma once
#ifndef SELECT_TOKEN_HEADER
#define SELECT_TOKEN_HEADER
#include <string>

enum select_token_type {
	SELECT_T, FROM, WHERE, ORDERBY, END
};

struct select_token {
	select_token_type type;
	std::string s;
};

#endif // !SELECT_TOKEN
