#include "drop_sql_parser.h"
#include "string_utils.h"
#include <stdexcept>

std::string drop_sql_parser::tb_name_from_line(std::string line) {
	string_utils::trim(line);
	if (line.find(' ') != std::string::npos ||
		line.find(')') != std::string::npos) {
		throw std::invalid_argument("Invalid table name: mustn't contain spaces or ')'.");
	}
	return line;
}