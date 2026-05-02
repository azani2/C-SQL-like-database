#include "select_tokenizer.h"
#include <stdexcept>

void select_tokenizer::skip_whitespaces() {
	while (pos < line.length() && (line[pos] == ' ' || line[pos] == '\t')) {
		pos++;
	}
}

select_tokenizer::select_tokenizer(const std::string& s) : line(s), pos(0) {}
select_token select_tokenizer::next() {
	skip_whitespaces();

	if (pos >= line.length()) {
		return { select_token_type::END, "" };
	}


	if (pos <= line.length() - 4 && line.substr(pos, 4) == "FROM") {
		pos += 4;
		if (line[pos] != ' ') {
			throw std::invalid_argument("Please separate FROM and the table name with a whitespace.");
		}
		skip_whitespaces();
		std::string tb_name_buf = "";
		while (pos < line.length()) {
			if (line[pos] == ' ' || line[pos] == '\t') {
				return select_token{ select_token_type::FROM, tb_name_buf };
			}
			tb_name_buf += line[pos];
			pos++;
		}
		return select_token{ select_token_type::FROM, tb_name_buf };
	}

	// "JOIN ON" can be added here but with it add more token types: joinon, colname, == (simple join with single strict key)

	if (pos <= line.length() - 5 && line.substr(pos, 5) == "WHERE") {
		pos += 5;
		if (line[pos] != ' ' && line[pos] != '\t') {
			throw std::invalid_argument("Please separate WHERE and the criteria with a whitespace.");
		}
		skip_whitespaces();
		std::string formula_buf = "";
		bool in_quotes = false;
		while (pos < line.length()) {
			char c = line[pos];
			if (c == '"' && formula_buf[formula_buf.length() - 1] != '\\') {
				in_quotes = !in_quotes;
			}
			if (!in_quotes && pos < line.length() - 9 && line.substr(pos, 9) == " ORDER BY") {
				return select_token{ select_token_type::WHERE, formula_buf };
			}
			formula_buf += c;
			pos++;
		}
		return select_token{ select_token_type::WHERE, formula_buf };
	}

	if (pos <= line.length() - 8 && line.substr(pos, 8) == "ORDER BY") {
		pos += 8;
		if (line[pos] != ' ' && line[pos] != '\t') {
			throw std::invalid_argument("Please separate ORDER BY clause and column names with a whitespace!");
		}
		skip_whitespaces();
		std::string colnames_to_order_buf = line.substr(pos);
		pos = line.length();
		return select_token{ select_token_type::ORDERBY, colnames_to_order_buf };
	}

	std::string colnames_to_select_buf = "";
	while (pos < line.length()) {
		if (pos < line.length() - 5 && line.substr(pos, 5) == " FROM") {
			return select_token{ select_token_type::SELECT_T, colnames_to_select_buf };
		}
		colnames_to_select_buf += line[pos];
		pos++;
	}
	return select_token{ select_token_type::SELECT_T, colnames_to_select_buf };

}