#include "database.h"
#include "insert_sql_parser.h"
#include "csv_parser.h"
#include "row_sorter.h"
#include "simple_sorter.h"
#include "value_parser.h"
#include "formula_evaluator.h"
#include "table_printer.h"
#include "row_hash_eq.h"
#include "column_types.h"
#include <unordered_set>

const size_t database::BIG_TABLE_ROW_CNT = 100;
const size_t database::PORTION_SIZE = 100;

database* database::instance = nullptr;

database::database() {};

database* database::get_instance() {
	if (!instance) {
		instance = new database();
		instance->load_tables_metadata();
	}
	return instance;
}

void database::load_tables_metadata() {
	disk_storage::prepare();
	data_integrity = true;
	try {
		disk_storage::data_integrity_check();
	}
	catch (std::exception& e) {
		data_integrity = false;
		throw;
	}
	std::vector<table> v;
	disk_storage::read_tables_metadata(v);
	for (table t : v) {
		tables.emplace(t.info.table_name, t);
	}
}

bool database::get_data_status() const {
	return data_integrity;
}

bool database::table_exists(const std::string& table_name) const {
	return (tables.count(table_name) != 0);
}

void database::assert_table_doesnt_exist(const std::string& table_name) const {
	if (table_exists(table_name)) {
		throw std::invalid_argument("Table with name " + table_name + " already exists in the database.");
	}
}

void database::assert_table_exists(const std::string & table_name) const {
	if (!table_exists(table_name)) {
		throw std::invalid_argument("Table with name " + table_name + " doesn't exist in the database.");
	}
}

void database::create_table(const table& t) {
	assert_table_doesnt_exist(t.info.table_name);
	disk_storage::save_table(t);
	tables.insert({ t.info.table_name, t });
}

void database::drop_table(const std::string& table_name) {
	assert_table_exists(table_name);
	disk_storage::delete_table(table_name);
	tables.erase(table_name);
}

void database::data_integrity_check() {
	try {
		disk_storage::data_integrity_check();
	}
	catch (std::exception& e) {
		data_integrity = false;
		throw;
	}
	data_integrity = true; //In the magic case where it was broken before and somehow fixed itself
}

void database::list_tables() const {
	size_t cnt = tables.size();
	std::string verb = (cnt > 1 ? "are" : "is");
	std::string noun = (cnt > 1 ? "tables" : "table");
	std::cout << " There " << verb << " " << cnt << " " << noun << " in the database.\n";// TODO: +DB_NAME и направи disk_storage да не е статичен и да може да му се задва работна папка (т.е. име на негоовата база данни). Може би даже го преименувай на database_disk_manager
	for (auto& p : tables) {
		std::cout << "\t" << p.first << std::endl;
	}
	std::cout << std::endl;
}

size_t database::insert(const std::string& table_name, const std::string& data) {
	assert_table_exists(table_name);

	table t_copy = tables.at(table_name);
	std::vector<row> to_insert;
	insert_sql_parser::rows_from_line_faster(data, t_copy, to_insert);

	std::vector<std::string> csv_rows;
	for (row curr : to_insert) {
		csv_rows.push_back(csv_parser::row_to_csv(curr));
	}

	bool indexed = t_copy.info.is_indexed;
	if (indexed) {
		t_copy.info.max_index = t_copy.info.max_index + csv_rows.size();
	}
	t_copy.info.row_count = t_copy.info.row_count + csv_rows.size();
	std::string csv_tbi = csv_parser::tb_info_to_csv(t_copy.info);

	size_t rows_inserted = disk_storage::save_rows(table_name, csv_rows, csv_tbi);

	tables.at(table_name).info.max_index = t_copy.info.max_index;;
	tables.at(table_name).info.row_count = t_copy.info.row_count;
	return rows_inserted;
}

size_t database::get_col_id(const table& t, const std::string& colname) const {
	size_t size = t.cols_info.size();
	for (size_t i = 0; i < size; i++) {
		if (colname == t.cols_info[i].col_name) {
			return i;
		}
	}
	throw std::invalid_argument("There is no column with the name " + colname + " in the table " + t.info.table_name);
}

void database::fill_formula_fields(formula* f, const table& t) const {
	if (!f) {
		return;
	}
	if (f->atomic) {
		f->col_idx = get_col_id(t, f->col_name);
		f->v = value_parser::from_string(f->value_str, t.cols_info[f->col_idx].type);
	}
	else {
		for (formula* child : f->children) {
			fill_formula_fields(child, t);
		}
	}
}

void database::validate_select_query(select_query& query, std::vector<size_t>& select_cols_ids, std::vector<size_t>& sort_cols_ids) const {
	std::string table_name = query.table_name;
	assert_table_exists(table_name);
	table t = tables.at(table_name);
	select_cols_ids.clear();
	for (auto& p : query.table_cols) {
		if (p.second == "*") {
			select_cols_ids.clear();
			break;
		}
		select_cols_ids.push_back(get_col_id(t, p.second));
	}
	simple_sorter<size_t>::sort(select_cols_ids);

	if (query.has_filter) {
		fill_formula_fields(query.filter, t);
	}

	if (query.sort) {
		sort_cols_ids.clear();
		for (auto& p : query.sort_cols) {
			sort_cols_ids.push_back(get_col_id(t, p.second));
		}
	}
}

void database::sort_rows(std::vector<row>& rows, const std::vector<size_t>& ids) const {
	row_sorter::sort(rows, ids);
}

//Ids must be sorted
void database::select_cols(std::vector<row>& rows, const std::vector<size_t>& ids) const {
	if (ids.empty()) {
		return;
	}
	size_t cols_size = rows.size();
	size_t ids_size = ids.size();
	for (size_t i = 0; i < cols_size; i++) {
		row temp;
		size_t sz = rows[i].size();
		size_t id_idx = 0;
		for (size_t j = 0; j < sz; j++) {
			if (id_idx >= ids_size) {
				break;
			}
			if (j == ids[id_idx]) {
				temp.push_back(rows[i][j]);
				id_idx++;
			}
		}
		rows[i] = temp;
	}
}

void database::filter_rows(std::vector<row>& rows, const formula* f) const {
	if (!f) return;
	std::vector<row>::iterator it = rows.begin();
	std::vector<row>::iterator end = rows.end();
	std::vector<row>::iterator curr = it;

	for (; curr != end; curr++) {
		if (formula_evaluator::eval(f, *curr)) {
			*it++ = std::move(*curr);
		}
	}
	rows.erase(it, end);
}

void database::keep_distinct(std::vector<row>& rows) const {
	std::unordered_set<row, row_hash, row_eq> elements;
	std::vector<row>::iterator it = rows.begin();
	std::vector<row>::iterator end = rows.end();
	std::vector<row>::iterator curr = it;

	for (; curr != end; curr++) {
		if (elements.insert(*curr).second) {
			*it++ = std::move(*curr);
		}
	}
	rows.erase(it, end);
}

void database::get_colnames(const std::vector<std::pair<std::string, std::string>>& cols, std::vector<std::string>& res) const {
	res.clear();
	for (auto& p : cols) {
		std::string colname = "";
		if (!p.first.empty()) {
			colname.append(p.first + ".");
		}
		colname.append(p.second);
		res.push_back(colname);
	}
}

void database::get_table_colnames(const table& t, std::vector<std::string>& res) const {
	res.clear();
	for (auto& col : t.cols_info) {
		res.push_back(col.col_name);
	}
}

size_t database::select(select_query& query) const {
	std::vector<size_t> select_cols_ids;
	std::vector<size_t> sort_cols_ids;
	validate_select_query(query, select_cols_ids, sort_cols_ids);
	std::string table_name = query.table_name;
	table t = tables.at(table_name);

	size_t row_count = t.info.row_count;
	size_t portions = row_count / database::PORTION_SIZE;
	if (row_count % PORTION_SIZE != 0) {
		portions++;
	}

	size_t filtered_rows_count = 0;

	disk_storage::prepare_query_file();

	for (size_t i = 1; i <= portions; i++) {
		size_t from = (i - 1) * PORTION_SIZE + 1;
		size_t to = i * PORTION_SIZE;
		if (to > row_count) {
			to = row_count;
		}
		std::vector<std::string> csv_rows;
		disk_storage::get_rows_from_to(table_name, csv_rows, from, to);

		std::vector<row> res_rows;
		for (auto& s : csv_rows) {
			res_rows.push_back(csv_parser::row_from_csv(s, t.cols_info));
		}

		if (query.sort) {
			sort_rows(res_rows, sort_cols_ids);
		}

		filter_rows(res_rows, query.filter);
		filtered_rows_count += res_rows.size();

		// If sorting is needed, save the portion to the disk to merge later
		if (query.sort) {
			std::vector<std::string> csv_res;
			for (auto& r : res_rows) {
				csv_res.push_back(csv_parser::row_to_csv(r));
			}
			disk_storage::save_sorting_portion(i, csv_res);
		}
		else { // Or save result
			select_cols(res_rows, select_cols_ids);
			
			std::vector<std::string> csv_res_rows;
			for (auto& r : res_rows) {
				csv_res_rows.push_back(csv_parser::row_to_csv(r));
			}
			disk_storage::save_query_res_portion(csv_res_rows);
		}
	}

	// Merge the sorted portions and save the result on the disk
	if (query.sort) {
		disk_storage::sort_portions(portions, sort_cols_ids, t.cols_info);
		for (size_t i = 1; i <= portions; i++) {
			bool last = (i == portions);
			size_t from = (i - 1) * PORTION_SIZE + 1;
			size_t to = i * PORTION_SIZE;
			if (to > filtered_rows_count) {
				to = filtered_rows_count;
			}

			std::vector<std::string> csv_res;
			disk_storage::get_query_sorting_res_from_to(csv_res, from, to, last, portions);
			std::vector<row> portion_rows;
			for (auto& csv_row : csv_res) {
				portion_rows.push_back(csv_parser::row_from_csv(csv_row, t.cols_info));
			}
			select_cols(portion_rows, select_cols_ids);
			
			std::vector<std::string> csv_res_rows;
			for (auto& r : portion_rows) {
				csv_res_rows.push_back(csv_parser::row_to_csv(r));
			}
			disk_storage::save_query_res_portion(csv_res_rows);
		}
	}

	if (query.distinct) {
		filtered_rows_count = disk_storage::remove_duplicates_in_query_res();
	}
	
	print_query_result(filtered_rows_count, t, select_cols_ids);
	return filtered_rows_count;
}

//Column ids need to be sorted!
std::vector<col_info> database::get_col_info_at(const table& t, const std::vector<size_t>& col_ids) const {
	if (col_ids.empty()) return t.cols_info;
	std::vector<col_info> res;
	size_t cols_size = t.cols_info.size();
	size_t ids_size = col_ids.size();
	size_t col_id_idx = 0;
	for (size_t i = 0; i < cols_size; i++) {
		if (col_id_idx >= ids_size) {
			break;
		}
		if (i == col_ids[col_id_idx]) {
			res.push_back(t.cols_info[i]);
			col_id_idx++;
		}
	}
	if (col_id_idx <= ids_size - 1) {
		throw std::logic_error("Invalid select column id: " + std::to_string(col_ids[col_id_idx]));
	}
	return res;
}

//Column ids need to be sorted!
std::vector<std::string> database::get_colnames_at(const table& t, const std::vector<size_t>& col_ids) const {
	std::vector<std::string> res;
	if (col_ids.empty()) {
		get_table_colnames(t, res);
		return res;
	}
	size_t cols_size = t.cols_info.size();
	size_t col_ids_size = col_ids.size();
	size_t col_id_idx = 0;
	for (size_t i = 0; i < cols_size; i++) {
		if (col_id_idx >= col_ids_size) {
			break;
		}
		if (i == col_ids[col_id_idx]) {
			res.push_back(t.cols_info[i].col_name);
			col_id_idx++;
		}
	}
	if (col_id_idx <= col_ids_size - 1) {
		throw std::logic_error("Invalid select column id: " + std::to_string(col_ids[col_id_idx]));
	}
	return res;
}

void database::print_query_result(const size_t rows_count, const table& t, const std::vector<size_t>& select_col_ids) const {
	size_t portions = rows_count / database::PORTION_SIZE;
	if (rows_count % PORTION_SIZE != 0) {
		portions++;
	}

	std::vector<col_info> selected_cols_info = get_col_info_at(t, select_col_ids);
	size_t cols_count = selected_cols_info.size();
	std::vector<size_t> col_sizes = disk_storage::get_query_res_max_col_lengths(cols_count, rows_count, selected_cols_info);
	table_printer printer(col_sizes);

	std::vector<std::string> select_colnames = get_colnames_at(t, select_col_ids);
	printer.print_header(select_colnames);

	for (size_t i = 1; i <= portions; i++) {
		bool last = (i == portions);
		size_t from = (i - 1) * PORTION_SIZE + 1;
		size_t to = i * PORTION_SIZE;
		if (to > rows_count) {
			to = rows_count;
		}

		std::vector<std::string> csv_res;
		disk_storage::get_query_res_from_to(csv_res, from, to);
		std::vector<row> portion_rows;
		for (auto& csv_row : csv_res) {
			portion_rows.push_back(csv_parser::row_from_csv(csv_row, selected_cols_info));
		}

		printer.print_rows(portion_rows);
	}
}

void database::table_info(const std::string& table_name) const {
	assert_table_exists(table_name);
	table t = tables.at(table_name);
	std::string description = " | ";
	for (auto& coli : t.cols_info) {
		description.append(coli.col_name);
		description.append(":");
		description.append(column_types::col_type_to_string(coli.type));
		if (coli.is_indexed) {
			description.append(", Indexed");
		}
		if (coli.has_default_value) {
			description.append(", Default ");
			description.append(value_parser::to_string(coli.default_value));
		}
		description.append(" | ");
	}

	std::cout << "Table " << table_name << " :" << std::endl;
	std::cout << description << std::endl;
	std::cout << "Total " << std::to_string(t.info.row_count) << " rows ";
	size_t size_on_disk = disk_storage::space_on_disk(table_name);
	std::cout << "( " << std::to_string(size_on_disk) << " KB data ) in the table" << std::endl;
}

size_t database::delete_where(const std::string& table_name, formula* f) {
	assert_table_exists(table_name);
	if (!f) {
		std::cout << "Are you sure you want to delete all rows from " << table_name << " ? y/n\n";

		std::string input;
		std::getline(std::cin, input);
		if (input == "n" || input == "N") {
			return 0;
		}
		if (input != "y" && input != "Y") {
			throw std::invalid_argument("Expected y, Y, n or N.");
		}
	}
	
	table t = tables.at(table_name);
	fill_formula_fields(f, t);
	size_t deleted_rows = disk_storage::delete_where(t.cols_info, f, t.info);
	return deleted_rows;
}