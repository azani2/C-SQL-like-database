#include "command_creator.h"
#include "string_utils.h"
#include <stdexcept>

std::pair<command_type, size_t> command_creator::get_type_from_line(const std::string& line) {
	size_t index = line.find(' ');
	if (index == std::string::npos) {
		index = line.length();
	}
	std::string comm_keyword = line.substr(0, index);
	string_utils::trim(comm_keyword);

	for (auto& p : command::COMMAND_STRINGS) {
		if (comm_keyword == p.second) {
			return { p.first, p.second.length() };
		}
	}

	std::string inv_comm = "";
	for (char c : line) {
		if (c == ' ') {
			break;
		}
		inv_comm += c;
	}
	throw std::invalid_argument("Invalid command. There is no command " + inv_comm + ".");
}

command command_creator::com_from_line(const std::string& line) {
	command res;
	auto type_pair = command_creator::get_type_from_line(line);
	res.type = type_pair.first;

	std::string data = "";
	if (line.length() > type_pair.second + 2) {
		data = line.substr(type_pair.second + 1, line.length() - type_pair.second);
		string_utils::remove_trailing_spaces(data);
	}
	res.data = data;
	
	return res;
}