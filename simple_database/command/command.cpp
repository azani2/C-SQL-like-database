#pragma once
#include "command.h"


const std::vector<std::pair<command_type, std::string>> command::COMMAND_STRINGS =
{ 
	{ CREATE, "CreateTable" }, 
	{ DROP, "DropTable" }, 
	{ INFO, "TableInfo" },
	{ SELECT, "Select" }, 
	{ REMOVE, "Remove" }, 
	{ INSERT, "Insert" },
	{ LIST, "ListTables" },
	{ CHECK, "CheckDataIntegrity" }
};