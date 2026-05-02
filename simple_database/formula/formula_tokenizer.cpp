#include "formula_tokenizer.h"
#include <stdexcept>

void formula_tokenizer::skip_whitespaces() {
	while (pos < line.length() && (line[pos] == ' ' || line[pos] == '\t')) {
		pos++;
	}
}

formula_tokenizer::formula_tokenizer(const std::string& s) : line(s), pos(0) {}

formula_token formula_tokenizer::next() {
	skip_whitespaces();

	if (pos >= line.length()) {
		return { formula_token_type::FT_END, "" };
	}

	char c = line[pos];

	if (c == '(') {
		pos++;
		return { formula_token_type::L_PARENTHESIS, "(" };
	}
	if (c == ')') {
		pos++;
		return { formula_token_type::R_PARENTHESIS, ")" };
	}

	if (c == '<') {
		pos++;
		return { formula_token_type::T_LESS, "<" };
	}
	if (c == '>') {
		pos++;
		return { formula_token_type::T_BIGGER, ">" };
	}
	if (c == '=' && pos + 1 < line.length() && line[pos + 1] == '=') {
		pos += 2;
		return { formula_token_type::T_EQUAL , "==" };
	}

	if (c == '"') {
		pos++;
		std::string s;
		while (pos < line.length()) {
			if (line[pos] == '"' && line[pos - 1] != '\\') {
				pos++;
				return { formula_token_type::VALUE, s };
			}
			if (line[pos] == '"' && line[pos - 1] == '\\') {
				s.pop_back();
			}
			s += line[pos];
			pos++;
		}
		throw std::invalid_argument("Unterminated value string - expected to be surrounded by \".");
	}

	size_t start = pos;
	while (pos < line.length() && line[pos] != ' ' && line[pos] != '\t') {
		pos++;
	}
	std::string s = line.substr(start, pos - start);

	if (s == "AND") {
		return { formula_token_type::T_AND, s };
	}
	if (s == "OR") {
		return { formula_token_type::T_OR, s };
	}
	if (s == "NOT") {
		return { formula_token_type::T_NOT, s };
	}

	return { formula_token_type::COLNAME, s };
}