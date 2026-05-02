#include "rel_parser.h"
#include <stdexcept>

const std::unordered_map<std::string, rel> rel_parser::FROM_STR = {
	{"<", LESS},
	{">", BIGGER},
	{"==", EQUAL}
};
const std::unordered_map<rel, std::string> rel_parser::TO_STR = {
	{LESS, "<"},
	{BIGGER, ">"},
	{EQUAL, "=="}
};

std::string rel_parser::rel_to_string(const rel& r) {
	if (!TO_STR.count(r)) {
		throw std::invalid_argument("Unknown rel.");
	}
	return TO_STR.at(r);
}

rel rel_parser::rel_from_string(const std::string& s) {
	if (!FROM_STR.count(s)) {
		throw std::invalid_argument("Unknown rel string: " + s);
	}
	return FROM_STR.at(s);
}