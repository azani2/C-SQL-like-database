#include "command_executor.h"
#include "create_sql_parser.h"
#include "database.h"
#include "drop_sql_parser.h"
#include "insert_sql_parser.h"
#include "select_sql_parser.h"
#include "string_utils.h"
#include "remove_sql_parser.h"
#include <iostream>

void command_executor::create_table(const command& com) const {
	table to_create = create_sql_parser::tb_from_line(com.data);
	database::get_instance()->create_table(to_create);
	std::cout << "Table " + to_create.info.table_name + " created!\n";
}

void command_executor::drop_table(const command& com) const {
	std::string table_name = drop_sql_parser::tb_name_from_line(com.data);
	database::get_instance()->drop_table(table_name); 
	std::cout << "Table " + table_name + " deleted!\n";
}

void command_executor::check_data_integrity() const {
	database::get_instance()->data_integrity_check();
	std::cout << "Data is OK.\n";
}

void command_executor::list_tables() const {
	database::get_instance()->list_tables();
}

void command_executor::insert(const command& comm) const {
	std::string table_name = insert_sql_parser::tb_name_from_line(comm.data);
	size_t rows_inserted = database::get_instance()->insert(table_name, comm.data);
	std::string noun = (rows_inserted > 1 ? " rows" : " row");
	std::cout << std::endl << std::to_string(rows_inserted) << noun << " inserted." << std::endl;
}

void command_executor::select(const command& comm) const {
	select_query query = select_sql_parser::from_line(comm.data);
	size_t rows_selected = database::get_instance()->select(query);
	std::string noun = (rows_selected > 1 ? " rows" : " row");
	std::cout << "\nTotal " + std::to_string(rows_selected) << noun << " rows selected.";
	std::cout << std::endl;
}

void command_executor::table_info(const command& comm) const {
	std::string table_name = comm.data;
	string_utils::trim(table_name);
	database::get_instance()->table_info(table_name);
	std::cout << std::endl;
}

void command_executor::remove(const command& comm) const {
	std::string table_name = remove_sql_parser::table_name_from_line(comm.data);
	formula* f = remove_sql_parser::formula_from_line(comm.data);
	size_t rows_deleted = database::get_instance()->delete_where(table_name, f);
	std::string noun = (rows_deleted > 1 ? " rows" : " row");
	std::cout << "\n " << std::to_string(rows_deleted) << noun << " deleted." << std::endl;
}

void command_executor::execute(const command& com) const {
	switch (com.type) {
	case CREATE:
		create_table(com);
		break;
	case DROP:
		drop_table(com);
		break;
	case INFO:
		table_info(com);
		break;
	case LIST:
		list_tables();
		break;
	case CHECK:
		check_data_integrity();
		break;
	case INSERT:
		insert(com);
		break;
	case SELECT:
		select(com);
		break;
	case REMOVE:
		remove(com);
		break;
	default:
		throw std::invalid_argument("Invalid command type.");
	}
}