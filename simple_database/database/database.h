#pragma once
#ifndef DATABASE_HEADER
#define DATABASE_HEADER
#include "table.h"
#include "row.h"
#include "disk_storage.h"
#include "formula_parser.h"
#include "formula_evaluator.h"
#include <unordered_map>
#include "select_query.h"

class database {
private:
	static const size_t BIG_TABLE_ROW_CNT;
	static const size_t PORTION_SIZE;

	// Metadata about all tables that exist in the database
	std::unordered_map<std::string, table> tables;
	bool data_integrity;

	void load_tables_metadata();

	database();
	static database* instance;

	bool table_exists(const std::string&) const;
	void assert_table_doesnt_exist(const std::string&) const;
	void assert_table_exists(const std::string&) const;
	void fill_formula_fields(formula*, const table&) const;
	void validate_select_query(select_query&, std::vector<size_t>&, std::vector<size_t>&) const;
	size_t get_col_id(const table&, const std::string&) const;
	void sort_rows(std::vector<row>&, const std::vector<size_t>&) const;
	void filter_rows(std::vector<row>&, const formula*) const;
	void select_cols(std::vector<row>&, const std::vector<size_t>&) const;
	void keep_distinct(std::vector<row>&) const;
	void get_table_colnames(const table&, std::vector<std::string>&) const;
	void get_colnames(const std::vector<std::pair<std::string, std::string>>&, std::vector<std::string>&) const;
	void print_query_result(const size_t, const table&, const std::vector<size_t>&) const;
	std::vector<col_info> get_col_info_at(const table&, const std::vector<size_t>&) const;
	std::vector<std::string> get_colnames_at(const table&, const std::vector<size_t>&) const;

public:
	database(const database&) = delete;
	static database* get_instance();

	bool get_data_status() const;

	void create_table(const table&);
	void drop_table(const std::string&);
	void data_integrity_check();
	void list_tables() const;
	void table_info(const std::string&) const;
	size_t insert(const std::string&, const std::string&);
	size_t delete_where(const std::string&, formula*);
	size_t select(select_query&) const;
	std::vector<row> join(const std::vector<row>&, const std::vector<row>&, const std::vector<std::pair<size_t, size_t>>&);
};

#endif
