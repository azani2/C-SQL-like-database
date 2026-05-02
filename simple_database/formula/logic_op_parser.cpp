#include "logic_op_parser.h"
#include <stdexcept>

const std::unordered_map<std::string, logic_op> logic_op_parser::FROM_STR = {
	{"NOT", NOT},
	{"AND", AND},
	{"OR", OR}
};

const std::unordered_map<logic_op, std::string> logic_op_parser::TO_STR = {
	{NOT, "NOT"},
	{AND, "AND"},
	{OR, "OR"}
};

logic_op logic_op_parser::from_string(const std::string& s) {
	if (!FROM_STR.count(s)) {
		throw std::invalid_argument("Unknown logic operator string: " + s);
	}
	return FROM_STR.at(s);
}

std::string logic_op_parser::to_string(const logic_op& op) {
	if (!TO_STR.count(op)) {
		throw std::invalid_argument("Unknown logic operator.");
	}
	return TO_STR.at(op);
}