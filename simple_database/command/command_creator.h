#pragma once

#ifndef COMMAND_CREATOR_HEADER
#define COMMAND_CREATOR_HEADER

#include "command.h"
#include <vector>

class command_creator {
private:
	command_creator() = delete;

	static std::pair<command_type, size_t> get_type_from_line(const std::string&);
public:
	static command com_from_line(const std::string&);
};

#endif