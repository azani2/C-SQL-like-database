#pragma once
#ifndef SELECT_TOKENIZER_HEADER
#define SELECT_TOKENIZER_HEADER
#include "select_token.h"

class select_tokenizer {
private:
	const std::string& line;
	size_t pos;

	void skip_whitespaces();
public:
	select_tokenizer(const std::string&);
	select_token next();
};

#endif // !SELECT_TOKENIZER_HEADER