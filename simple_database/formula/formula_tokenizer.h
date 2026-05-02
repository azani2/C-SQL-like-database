#pragma once
#ifndef FORMULA_TOKENIZER_HEADER
#define FORMULA_TOKENIZER_HEADER
#include "formula_token.h"

class formula_tokenizer {
private:
	const std::string& line;
	size_t pos;

	void skip_whitespaces();
public:
	formula_tokenizer(const std::string&);
	formula_token next();
};

#endif // !TOKENIZER_HEADER


