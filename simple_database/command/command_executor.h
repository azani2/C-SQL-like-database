#pragma once
#ifndef COMMAND_EXECUTOR_HEADER
#define COMMAND_EXECUTOR_HEADER
#include "command.h"
#include "table.h"

class command_executor {
private:
	void create_table(const command&) const;
	void drop_table(const command&) const;
	void check_data_integrity() const;
	void list_tables() const;
	void table_info(const command&) const;
	void select(const command&) const;
	void remove(const command&) const;
	void insert(const command&) const;

public:
	void execute(const command&) const;
	void wrapitup() {};
};

#endif
