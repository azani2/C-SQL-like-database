#pragma once
#ifndef LOGIC_OP_PARSER_HEADER
#define LOGIC_OP_PARSER_HEADER
#include <string>
#include "logic_op.h"
#include <unordered_map>

class logic_op_parser {
private:
	static const std::unordered_map<std::string, logic_op> FROM_STR;
	static const std::unordered_map<logic_op, std::string> TO_STR;
public:
	static logic_op from_string(const std::string&);
	static std::string to_string(const logic_op&);
};

#endif // !LOGIC_OP_PARSER_HEADER


