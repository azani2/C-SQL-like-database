#include "string_utils.h"
#include <stdexcept>

void string_utils::remove_trailing_spaces(std::string& s) {
	size_t idx_first = s.find_first_not_of(' ');
	if (idx_first > 0 && idx_first != std::string::npos) {
		s.erase(0, idx_first);
	}
	size_t idx_last = s.find_last_not_of(' ');
	if (idx_last < s.length() - 1) {
		s.erase(idx_last + 1, s.length() - 1 - idx_last);
	}
	if (idx_first == std::string::npos && idx_last == std::string::npos) {
		s.clear();
	}
}

void string_utils::trim(std::string& s) {
	bool after_space = true;
	size_t len = s.length();
	for (size_t i = 0; i < len; i++) {
		if (s[i] == ' ') {
			if (!after_space) {
				after_space = true;
			}
			else {
				s.erase(i, 1);
				i--;
				len--;
			}
		}
		else {
			after_space = false;
		}
	}
	if (!s.empty() && s[s.length() - 1] == ' ') {
		s.erase(s.length() - 1, 1);
	}
}

void string_utils::unquote(std::string& s) {
	size_t first_quote = s.find('"');
	size_t last_quote = s.find_last_of('"');
	if (first_quote == std::string::npos || last_quote == std::string::npos) {
		return;
	}
	//if there is text before or after "..." then the text is not quoted, just contains quotes
	for (size_t i = 0; i < first_quote; i++) {
		if (s[i] != ' ') {
			return;
		}
	}
	for (size_t i = last_quote + 1; i < s.length(); i++) {
		if (s[i] != ' ') {
			return;
		}
	}
	s.erase(last_quote, 1);
	s.erase(first_quote, 1);
}

std::string string_utils::size_t_to_string_4_digit(const size_t& sz) {
	if (sz > 9999) {
		throw std::invalid_argument("Number cannot be writen in 2 digits.");
	}
	if (sz < 10) {
		return ("0" + std::to_string(sz));
	} 
	if (sz < 100) {
		return ("00" + std::to_string(sz));
	}
	if (sz < 1000) {
		return ("000" + std::to_string(sz));
	}
	return std::to_string(sz);
}

std::string string_utils::size_t_to_string_2_digit(const size_t& sz) {
	if (sz > 99) {
		throw std::invalid_argument("Number cannot be writen in 2 digits.");
	}
	if (sz < 10) {
		return ("0" + std::to_string(sz));
	}
	return std::to_string(sz);
}

std::string string_utils::date_to_string(const date& d) {
	return std::to_string(d.year) + "-"
		+ size_t_to_string_2_digit(d.month) + "-"
		+ size_t_to_string_2_digit(d.day);
}

std::string string_utils::datetime_to_string(const datetime& dt) {
	return date_to_string(dt.dt) + " "
		+ size_t_to_string_2_digit(dt.hr) + ":"
		+ size_t_to_string_2_digit(dt.min) + ":"
		+ size_t_to_string_2_digit(dt.sec);
}