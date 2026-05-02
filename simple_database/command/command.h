#pragma once
#include "command_type.h"
#include <string>
#include <vector>

struct command {
	static const std::vector<std::pair<command_type, std::string>> COMMAND_STRINGS;

	command_type type;
	std::string data;
};
