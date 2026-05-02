#pragma once
#ifndef REL_PARSER_HEADER
#define REL_PARSER_HEADER
#include "rel.h"
#include <string>
#include <unordered_map>

class rel_parser {
private:
	static const std::unordered_map<std::string, rel> FROM_STR;
	static const std::unordered_map<rel, std::string> TO_STR;
public:
	static std::string rel_to_string(const rel&);
	static rel rel_from_string(const std::string&);
};


#endif // !REL_PARSER_HEADER


