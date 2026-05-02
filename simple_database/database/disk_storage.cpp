#include "disk_storage.h"
#include "csv_parser.h"
#include "crc32.h"
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include "rows_binary_heap.h"
#include "value_parser.h"
#include <unordered_set>
#include <unordered_map>
#include <functional>

const size_t disk_storage::BUF_FLUSH_THRESHOLD = 2000;

const int disk_storage::KB_SIZE = 1024;
const int disk_storage::MAX_CACHE_SIZE = 1024;

const std::string disk_storage::DB_NAME = "db";
const std::string disk_storage::DB_PATH = DB_NAME + "\\";
const std::string disk_storage::QUERY_NAME = "queries";
const std::string disk_storage::QUERY_PATH = "queries\\";
const std::string disk_storage::TABLES_CNT_FILE_NAME = "tables_count";
const std::string disk_storage::TABLES_INFO_FILE_NAME = "tables_info";
const std::string disk_storage::SEEN_FILE_NAME = "seen";
const std::string disk_storage::QUERY_SORTED = "query_sorted";
const std::string disk_storage::QUERY_RES = "query_res";
const std::string disk_storage::QUERY_PORTION_NAME = "temp";
const std::string disk_storage::CSV = ".csv";
const std::string disk_storage::TXT = ".txt";
const std::string disk_storage::DAT = ".dat";
const std::string disk_storage::CHCK = "_chck";
const std::string disk_storage::COLS = "_cols";
const std::string disk_storage::OLD = "_old";
const std::string disk_storage::NEW = "_new";

std::string disk_storage::get_seen_file() {
	return DB_PATH + QUERY_PATH + SEEN_FILE_NAME;
}

std::string disk_storage::get_tables_count_file() {
	return DB_PATH + TABLES_CNT_FILE_NAME;
}

std::string disk_storage::get_tables_info_file() {
	return DB_PATH + TABLES_INFO_FILE_NAME;
}

std::string disk_storage::get_query_sorted_file_full() {
	return	DB_PATH + QUERY_PATH + QUERY_SORTED;
}

std::string disk_storage::get_query_res_file_full() {
	return	DB_PATH + QUERY_PATH + QUERY_RES;
}

std::string disk_storage::get_query_sorted_file() {
	return QUERY_PATH + QUERY_SORTED;
}

std::string disk_storage::get_query_res_file() {
	return	QUERY_PATH + QUERY_RES;
}

std::string disk_storage::get_portion_file(const size_t p_idx) {
	return DB_PATH + QUERY_PATH + QUERY_PORTION_NAME + std::to_string(p_idx);
}

std::string disk_storage::get_old_file(const std::string& filename) {
	return filename + OLD;
}

std::string disk_storage::get_cols_file(const std::string& table_name) {
	return table_name + COLS;
}

std::string disk_storage::get_chck_file(const std::string& filename) {
	return filename + CHCK;
}

void disk_storage::prepare() {
	struct stat db;
	if (stat(DB_NAME.data(), &db) != 0) {
		std::filesystem::create_directory(DB_NAME);
	}
	struct stat dbq;
	if (stat((DB_PATH + QUERY_NAME).data(), &dbq) != 0) {
		std::filesystem::create_directory(DB_PATH + QUERY_NAME);
	}
	if (!file_exists(get_tables_count_file() + TXT)) {
		create_if_not_exists(get_tables_count_file() + TXT);
		calculate_and_save_crcs_for(get_tables_count_file(), TXT);
	}
	if (!file_exists(get_tables_info_file() + CSV)) {
		create_if_not_exists(get_tables_info_file() + CSV);
		calculate_and_save_crcs_for(get_tables_info_file(), CSV);
	}
}

bool disk_storage::file_exists(const std::string& full_filename) {
	std::ifstream fin(full_filename);
	bool exists = fin.good();
	fin.close();
	return exists;
}

bool disk_storage::is_empty_file(const std::string& full_filename) {
	if (!file_exists(full_filename)) return 0;
	std::ifstream fin(full_filename);
	bool is_empty = (fin.peek() == std::ifstream::traits_type::eof());
	fin.close();
	return is_empty;
}

void disk_storage::create_if_not_exists(const std::string& full_filename) {
	std::ifstream fin(full_filename);
	if (!fin.good()) {
		fin.close();
		std::fstream fs(full_filename, std::ios::out | std::ios::app);
		if (!fs.good()) {
			fs.close();
			throw std::ios_base::failure("Error creating file " + full_filename + " .");
		}
		fs.close();
	}
	fin.close();
}

void disk_storage::delete_files(const std::vector<std::string>& to_remove) { 
	for (std::string filename : to_remove) {
		std::remove(filename.data());
	}
}

void disk_storage::backup_file(const std::string& filename, const std::string& extension) {
	std::string fn_old = get_old_file(filename) + extension;
	std::string fn_act = filename + extension;
	if (!file_exists(fn_act)) {
		throw std::logic_error("File " + fn_act + " doesn't exist.");
	}
	rename_to_old(filename, extension);
	if (is_empty_file(fn_old)) {
		create_if_not_exists(fn_act);
		return;
	}

	std::fstream fout(fn_act, std::ios::out | std::ios::app);
	if (!fout.good()) {
		fout.close();
		restore_from_old(filename, extension);
		throw std::ios_base::failure("Error opening file " + fn_act + " for writing.");
	}
	std::ifstream fin(fn_old);
	if (!fout.good()) {
		fin.close();
		fout.close();
		restore_from_old(filename, extension);
		throw std::ios_base::failure("Error opening file " + fn_act + " for reading.");
	}
	
	std::string line;
	while (std::getline(fin, line)) {
		fout << line << std::endl;
	}
	fin.close();
	fout.close();
}

void disk_storage::rename_to_old(const std::string& filename, const std::string& extension) {
	std::string old = get_old_file(filename) + extension;
	std::string actual = filename + extension;
	std::error_code ec;
	std::filesystem::rename(actual, old, ec);
	if (ec) {
		throw std::ios_base::failure("Error renaming " + actual + " to " + old + " : " + ec.message());
	}
}

void disk_storage::restore_from_old(const std::string& filename, const std::string& ext) {
	std::string old = get_old_file(filename) + ext;
	std::string actual = filename + ext;
	if (!file_exists(old)) {
		throw std::logic_error("Cannot recover file because " + old + " doesn't exist.");
	}
	if (file_exists(actual)) {
		std::remove(actual.data());
	}
	int res = std::rename(old.data(), actual.data());
	if (res != 0) {
		throw std::ios_base::failure("Error renaming " + old + " to " + actual + " .");
	}
}

void disk_storage::restore_files(const std::vector<std::pair<std::string, std::string>>& to_restore) {
	for (auto& p : to_restore) {
		restore_from_old(p.first, p.second);
	}
}

void disk_storage::delete_safe(const std::string& filename, const std::string& extension,
	const std::vector<std::pair<std::string, std::string>>& to_restore) {
	try {
		rename_to_old(filename, extension);
	}
	catch (std::exception& e) {
		restore_files(to_restore);
		throw;
	}
}

void disk_storage::calculate_crcs(const std::string& full_filename, std::vector<size_t>& v) {
	if (!file_exists(full_filename)) {
		throw std::logic_error("Cannot calculate crc codes for file " + full_filename + " because it doesn't exist.");
	}
	v.clear();
	if (is_empty_file(full_filename)) {
		v.push_back(0);
		return;
	}

	std::ifstream fin(full_filename, std::ios::binary);
	if (!fin) {
		fin.close();
		throw std::ios_base::failure("Error opening " + full_filename + " for reading.");
	}

	char buf[4096];
	while (fin) {
		fin.read(buf, sizeof(buf));
		std::streamsize bytes_read = fin.gcount();
		if (bytes_read != 0) {
			size_t crc = crc32::generate_crc32_fast(buf, bytes_read);
			v.push_back(crc);
		}
	}
	fin.close();
}

void disk_storage::save_crcs(const std::string& filename, const std::vector<size_t>& crcs) {
	if (filename.empty()) {
		throw std::invalid_argument("Invalid filename " + filename + " .");//TODO: add better filename validation
	}
	if (crcs.empty()) {
		throw std::invalid_argument("Crc array is empty!");
	}

	std::string chck_f = get_chck_file(filename) + TXT;
	create_if_not_exists(chck_f);
	std::ofstream fout(chck_f, std::ios::trunc);
	if (!fout.good()) {
		fout.close();
		throw std::ios_base::failure("Error opening checksums file for " + filename + ".");
	}

	fout << crcs.size() << std::endl;
	for (size_t crc : crcs) {
		fout << crc << std::endl;
	}
	fout.close();
}

void disk_storage::calculate_and_save_crcs_for(const std::string& filename, const std::string& extension) {
	std::vector<size_t> crcs;
	calculate_crcs(filename + extension, crcs);
	save_crcs(filename, crcs);
}

void disk_storage::read_crcs(const std::string& filename, std::vector<size_t>& v) {
	std::string chck_file = get_chck_file(filename) + TXT;

	std::ifstream fin(chck_file);
	if (!fin.good()) {
		fin.close();
		throw std::ios_base::failure("Error opening checksum file for " + filename + ".");
	}

	size_t crc_cnt;
	fin >> crc_cnt;
	for (size_t i = 0; i < crc_cnt; i++) {
		if (!fin.good()) {
			fin.close();
			throw std::ios_base::failure("Data integriy check failed."); // TODO
		}
		size_t curr_crc;
		fin >> curr_crc;
		v.push_back(curr_crc);
	}
	fin.close();
}

void disk_storage::data_integrity_check_file(const std::string& filename, const std::string& extension) {
	std::vector<size_t> saved_crcs;
	read_crcs(filename, saved_crcs);
	size_t count = saved_crcs.size();

	std::vector<size_t> calculated_crcs;
	calculate_crcs(filename + extension, calculated_crcs);

	if (count != calculated_crcs.size()) {
		throw std::logic_error("Data integrity check failed."); // TODO: обяснения на грешките
	}
	for (size_t i = 0; i < count; i++) {
		if (saved_crcs[i] != calculated_crcs[i]) {
			throw std::logic_error("Data integrity check failed."); // TODO: обяснеия на грешките
		}
	}
}

void disk_storage::data_integrity_check() {
	data_integrity_check_file(get_tables_count_file(), TXT);
	data_integrity_check_file(get_tables_info_file(), CSV);

	std::vector<std::string> tb_names;
	read_tables_names(tb_names);
	for (std::string tbn : tb_names) {
		data_integrity_check_file(DB_PATH + tbn, CSV);
		data_integrity_check_file(DB_PATH + get_cols_file(tbn), CSV);
	}
}

void disk_storage::save_tables_count(size_t count) {
	std::ofstream fout(get_tables_count_file() + TXT, std::ios::trunc);
	if (!fout.good()) {
		fout.close();
		throw std::ios_base::failure("Couldn't open file " + TABLES_CNT_FILE_NAME + " .");
	}
	fout << count << std::endl;
	fout.close();
}

size_t disk_storage::read_tables_count() {
	if (is_empty_file(get_tables_count_file() + TXT)) {
		save_tables_count(0);
		return 0;
	}
	std::ifstream fin(get_tables_count_file() + TXT);
	if (!fin.good()) {
		fin.close();
		throw std::ios_base::failure("Couldn't open file " + TABLES_CNT_FILE_NAME + " .");
	}
	size_t cnt;
	fin >> cnt;
	fin.close();
	return cnt;
}

void disk_storage::decrement_tables_count() {
	size_t cnt = read_tables_count();
	if (cnt == 0) {
		throw std::logic_error("Current tables count is 0. Cannot decrease further.");
	}
	save_tables_count(cnt - 1);
}

void disk_storage::increment_tables_count() {
	size_t cnt = read_tables_count();
	save_tables_count(cnt + 1);
}

void disk_storage::read_tables_names(std::vector<std::string>& v) {
	v.clear();
	size_t cnt = read_tables_count();
	if (cnt == 0) return;

	std::ifstream fin(get_tables_info_file() + CSV);
	if (!fin.good()) {
		fin.close();
		throw std::ios_base::failure("Failed to open " + get_tables_info_file() + CSV + " for reading.");
	}

	for (size_t i = 0; i < cnt; i++) {
		std::string row;
		if (!std::getline(fin, row) || row.empty()) {
			fin.close();
			throw std::logic_error("Saved tables count doesn't match real count of tables recorded in database.");
		}
		std::string tb_name = csv_parser::tb_name_from_csv(row);
		v.push_back(tb_name);
	}
}

void disk_storage::read_cols_info(const std::string& table_name, const size_t col_count, std::vector<col_info>& v) {
	if (!file_exists(DB_PATH + get_cols_file(table_name) + CSV)) {
		throw std::logic_error("Coulmn metadata file for table " + table_name + " doesn't exist.");
	}

	v.clear();
	std::ifstream fin(DB_PATH + get_cols_file(table_name) + CSV);
	if (!fin.good()) {
		fin.close();
		throw std::ios_base::failure("Error opening file " + DB_PATH + table_name + " for reading.");
	}

	for (size_t i = 0; i < col_count; i++) {
		std::string line;
		if (!std::getline(fin, line) || line.empty()) { // TODO: better exceptions: how many? which table? add more informations to other exceptions too
			fin.close();
			throw std::logic_error("Saved table column count doesn't match real count of columns described in database.");
		}
		v.push_back(csv_parser::col_info_from_csv(line));
	}
	fin.close();
}

void disk_storage::read_table_metadata(const size_t tables_cnt, const std::string& table_name, std::vector<col_info>& v) {
	std::ifstream fin(get_tables_info_file() + CSV);
	if (!fin.good()) {
		fin.close();
		throw std::ios_base::failure("Failed to open " + get_tables_info_file() + CSV + " for reading.");
	}

	table_info tbi;
	bool found = false;
	for (size_t i = 0; i < tables_cnt; i++) {
		std::string line;
		if (!std::getline(fin, line) || line.empty()) {
			fin.close();
			throw std::logic_error("Saved tables count doesn't match real count of tables recorded in database.");
		}
		if (csv_parser::tb_name_from_csv(line) == table_name) {
			tbi = csv_parser::tb_info_from_csv(line);
			found = true;
			break;
		}
	}
	fin.close();
	if (!found) {
		throw std::invalid_argument("No tables with the name " + table_name + " is saved on the disk.");
	}
	read_cols_info(table_name, tbi.col_count, v);
}

void disk_storage::read_tables_metadata(std::vector<table>& v) {
	v.clear();
	size_t tables_cnt = read_tables_count();
	if (tables_cnt == 0) return;

	std::ifstream fin(get_tables_info_file() + CSV);
	if (!fin.good()) {
		fin.close();
		throw std::ios_base::failure("Error opening file " + get_tables_info_file() + CSV + " for reading.");
	}

	for (size_t i = 0; i < tables_cnt; i++) {
		std::string line;
		if (!std::getline(fin, line) || line.empty()) {
			fin.close();
			throw std::logic_error("Saved tables count " + std::to_string(tables_cnt) + " doesn't match real count of tables recorded in database: " + std::to_string(i) + ".");
		}
		table_info tbi = csv_parser::tb_info_from_csv(line);
		std::vector<col_info> colsi;
		read_cols_info(tbi.table_name, tbi.col_count, colsi);
		v.emplace_back(tbi, colsi);
	}
	fin.close();
}

void disk_storage::save_cols_info(const table& t) {
	std::string coli_fn = DB_PATH + get_cols_file(t.info.table_name) + CSV;
	std::ofstream fout(coli_fn, std::ios::trunc);
	if (!fout.good()) {
		throw std::ios_base::failure("Unable to open file " + coli_fn + " .");
	}
	size_t cnt = 0;
	for (col_info col : t.cols_info) {
		fout << csv_parser::col_info_to_csv(col) << std::endl;
		cnt++;
	}
	fout.close();
	if (cnt != t.cols_info.size()) {
		std::remove(coli_fn.data());
		throw std::logic_error("Error saving column info data to " + coli_fn + ", reverted.");
	}
}

void disk_storage::save_tb_info(const table_info& tbi) {
	std::ofstream fout(get_tables_info_file() + CSV, std::ios::app);
	fout << csv_parser::tb_info_to_csv(tbi) << std::endl;
	fout.close();
}

void disk_storage::save_table(const table& t) { 
	std::vector<std::string> to_delete_if_fail;

	//Create empty rows file
	std::string fn = DB_PATH + t.info.table_name + CSV;
	create_if_not_exists(fn);
	
	to_delete_if_fail.push_back(fn);

	// Create empty column description file
	fn = DB_PATH + get_cols_file(t.info.table_name) + CSV;
	try {
		create_if_not_exists(fn);
	}
	catch (std::exception& e) {
		delete_files(to_delete_if_fail);
		throw;
	}
	to_delete_if_fail.push_back(fn);

	// Save column descriptions to table_name_cols.csv
	try {
		save_cols_info(t);
	}
	catch (std::exception& e) {
		delete_files(to_delete_if_fail);
		throw;
	}

	// Save checksums for table_name_cols.csv
	fn = DB_PATH + get_chck_file(get_cols_file(t.info.table_name)) + TXT;
	try {
		calculate_and_save_crcs_for(DB_PATH + get_cols_file(t.info.table_name), CSV);
	}
	catch (std::exception& e) {
		delete_files(to_delete_if_fail);
		if (file_exists(fn)) {
			std::remove(fn.data());
		}
		throw;
	}
	to_delete_if_fail.push_back(fn);

	// Save checksums (0) for table_name.csv (rows file but there is no rows yet)
	fn = DB_PATH + get_chck_file(t.info.table_name) + TXT;
	try {
		calculate_and_save_crcs_for(DB_PATH + t.info.table_name, CSV);
	}
	catch (std::exception& e) {
		delete_files(to_delete_if_fail);
		if (file_exists(fn)) {
			std::remove(fn.data());
		}
		throw;
	}
	to_delete_if_fail.push_back(fn);

	// Backup tables_info.csv
	try {
		backup_file(get_tables_info_file(), CSV);
	}
	catch (std::exception& e) {
		if (file_exists(get_old_file(get_tables_info_file()) + CSV)) {
			std::remove((get_old_file(get_tables_info_file()) + CSV).data());
		}
		delete_files(to_delete_if_fail);
		throw;
	}
	to_delete_if_fail.push_back(get_tables_info_file() + CSV);

	// Append table metadata to tables_info.csv
	try {
		save_tb_info(t.info);
	}
	catch (std::exception& e) {
		delete_files(to_delete_if_fail);
		restore_from_old(get_tables_info_file(), CSV);
		throw;
	}

	// Backup old checksum file for tables_info.csv
	std::string tbi_chck_f = get_chck_file(get_tables_info_file());
	try {
		backup_file(tbi_chck_f, TXT);
	}
	catch (std::exception& e) {
		delete_files(to_delete_if_fail);
		restore_from_old(get_tables_info_file(), CSV);
		throw;
	}
	to_delete_if_fail.push_back(tbi_chck_f + TXT);

	// Calculate crcs for tables_info.csv and save to new file
	try {
		calculate_and_save_crcs_for(get_tables_info_file(), CSV);
	}
	catch (std::exception& e) {
		delete_files(to_delete_if_fail);
		restore_from_old(get_tables_info_file(), CSV);
		restore_from_old(tbi_chck_f, TXT);
		throw;
	}
	to_delete_if_fail.push_back(fn + TXT);

	// Edit tables_count.txt file
	size_t old_tb_cnt = read_tables_count();
	try {
		increment_tables_count();
	}
	catch (std::exception& e) {
		save_tables_count(old_tb_cnt);
		delete_files(to_delete_if_fail);
		restore_from_old(get_tables_info_file(), CSV);
		restore_from_old(tbi_chck_f, TXT);
		throw;
	}

	// Backup ofd crc file for tables_count.txt
	std::string tbc_chck_f = get_chck_file(get_tables_count_file());
	try {
		backup_file(tbc_chck_f, TXT);
	}
	catch (std::exception& e) {
		delete_files(to_delete_if_fail);
		throw;
	}
	to_delete_if_fail.push_back(tbc_chck_f + TXT);

	// Calculate crcs for tables_count.txt and save to new file
	try {
		calculate_and_save_crcs_for(get_tables_count_file(), TXT);
	}
	catch (std::exception& e) {
		save_tables_count(old_tb_cnt);
		delete_files(to_delete_if_fail);
		restore_from_old(get_tables_info_file(), CSV);
		restore_from_old(tbi_chck_f, TXT);
		restore_from_old(tbc_chck_f, TXT);
		throw;
	}

	std::remove((get_old_file(get_tables_info_file()) + CSV).data());
	std::remove((get_old_file(get_chck_file(get_tables_info_file())) + TXT).data());
	std::remove((get_old_file(get_chck_file(get_tables_count_file())) + TXT).data());
}

void disk_storage::copy_tbi_rows_except(const std::string& table_name) {
	std::string act = get_tables_info_file();
	std::string old = get_old_file(act);
	if (!file_exists(old + CSV)) {
		throw std::logic_error("Cannot delete table info row beacuse file " + old + CSV + " doesn't exist.");
	}
	create_if_not_exists(act + CSV);
	std::ofstream dest(act + CSV);
	std::ifstream src(old + CSV);

	if (!dest.good()) {
		throw std::ios_base::failure("Error opening file " + act + " for writing .");
	}
	if (!src.good()) {
		throw std::ios_base::failure("Error opening file " + old + " for reading .");
	}

	size_t cnt = read_tables_count();
	for (size_t i = 0; i < cnt; i++) {
		if (!src.good()) {
			throw std::logic_error("Saved tables count doesn't match actual count of tables described in " + old + " .");
		}
		std::string line;
		std::getline(src, line);
		if (line.empty()) {
			throw std::ios_base::failure("Error reading from file " + old + " .");
		}
		size_t idx = line.find(',');
		if (idx == std::string::npos) {
			throw std::logic_error("Saved table info data in " + old + " is not in the right format." + 
				"\nCannot extract table name from line " + std::to_string(i) + ":" +
				"\n" + line + "\n");
		}
		if (line.substr(0, idx) == table_name) {
			continue;
		}
		dest << line << std::endl;
	}
	// TODO: if  (i == cnt && src.good()) => има още редове, а не би трябвало!
	dest.close(); 
	src.close();
}

void disk_storage::delete_table(const std::string& table_name) {
	std::string act_fn = get_tables_info_file();
	std::string old_fn = get_old_file(act_fn);
	std::vector<std::pair<std::string, std::string>> to_restore_if_fail;
	std::vector<std::string> old_to_delete;

	// tables_info.csv -> tables_info.old.csv
	rename_to_old(act_fn, CSV);
	to_restore_if_fail.emplace_back(act_fn, CSV);
	old_to_delete.push_back(old_fn + CSV);

	try {
		copy_tbi_rows_except(table_name);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	// tables_info_chck.txt -> tables_info_chck_old.txt
	act_fn = get_chck_file(act_fn);
	old_fn = get_old_file(act_fn);
	try {
		rename_to_old(act_fn, TXT);
	}
	catch (std::exception& e) {
		if (!file_exists(act_fn + TXT)) {
			to_restore_if_fail.emplace_back(act_fn, TXT);
		}
		restore_files(to_restore_if_fail);
		throw;
	}
	to_restore_if_fail.emplace_back(act_fn, TXT);
	old_to_delete.push_back(old_fn + TXT);

	// Recalculate table_info.csv checksums:
	try {
		calculate_and_save_crcs_for(get_tables_info_file(), CSV);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	// TODO: Code duplication
	// table_name_cols.csv -> table_name_cols.csv
	act_fn = DB_PATH + get_cols_file(table_name);
	to_restore_if_fail.emplace_back(act_fn, CSV);
	delete_safe(act_fn, CSV, to_restore_if_fail);
	old_to_delete.push_back(get_old_file(act_fn) + CSV);

	// table_name_cols_chck.txt -> table_name_cols_chck.txt
	act_fn = DB_PATH + get_chck_file(get_cols_file(table_name));
	to_restore_if_fail.emplace_back(act_fn, TXT);
	delete_safe(act_fn, TXT, to_restore_if_fail);
	old_to_delete.push_back(get_old_file(act_fn) + TXT);

	// table_name.csv -> table_name_old.csv
	to_restore_if_fail.emplace_back(DB_PATH + table_name, CSV);
	delete_safe(DB_PATH + table_name, CSV, to_restore_if_fail);
	old_to_delete.push_back(DB_PATH + get_old_file(table_name) + CSV);

	// table_name_chck.csv -> table_name_old_chck.csv
	act_fn = DB_PATH + get_chck_file(table_name);
	to_restore_if_fail.emplace_back(act_fn, TXT);
	delete_safe(act_fn, TXT, to_restore_if_fail);
	old_to_delete.push_back(get_old_file(act_fn) + TXT);

	size_t tb_cnt = read_tables_count();
	try {
		decrement_tables_count();
	}
	catch (std::exception& e) {
		save_tables_count(tb_cnt);
		restore_files(to_restore_if_fail);
		throw;
	}

	// tables_count_chck.txt -> tables_count_chck_old.txt
	act_fn = get_chck_file(get_tables_count_file());
	old_fn = get_old_file(act_fn);
	try {
		rename_to_old(act_fn, TXT);
	}
	catch (std::exception& e) {
		if (!file_exists(act_fn + TXT)) {
			to_restore_if_fail.emplace_back(act_fn, TXT);
		}
		restore_files(to_restore_if_fail);
		throw;
	}
	to_restore_if_fail.emplace_back(act_fn, TXT);
	old_to_delete.push_back(old_fn + TXT);

	// Recalculate table_info.csv checksums:
	try {
		calculate_and_save_crcs_for(get_tables_count_file(), TXT);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	// If all went well delete _old files
	delete_files(old_to_delete);
}

size_t disk_storage::add_new_rows(const std::string& full_filename, const std::vector<std::string>& rows) {
	std::ofstream fout(full_filename, std::ios::app);
	if (!fout.good()) {
		throw std::ios_base::failure("Error opening file " + full_filename + " for writing .");
	}
	size_t rows_added = 0;
	for (auto& row : rows) {
		fout << row << std::endl;
		rows_added++;
	}
	fout.close();
	return rows_added;
}

size_t disk_storage::save_rows(const std::string& table_name, const std::vector<std::string>& rows, const std::string& tb_info) {
	std::vector<std::pair<std::string, std::string>> to_restore_if_fail;
	std::vector<std::string> to_delete;

	// Backup table_name.csv to table_name_old.csv
	std::string tbf = DB_PATH + table_name;
	try {
		backup_file(tbf, CSV);
	}
	catch (std::exception& e) {
		restore_from_old(tbf, CSV);
		throw;
	}
	to_restore_if_fail.emplace_back(tbf, CSV);
	to_delete.push_back(get_old_file(tbf) + CSV);

	// Add rows to table_name.csv
	size_t rows_added = 0;
	try {
		rows_added = add_new_rows(tbf + CSV, rows);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	// Rename table_name_chck.txt to table_name_chck_old.txt
	std::string tb_chckf = get_chck_file(tbf);
	try {
		rename_to_old(tb_chckf, TXT);
	}
	catch (std::exception& e) {
		restore_from_old(tb_chckf, TXT);
		restore_files(to_restore_if_fail);
		throw;
	}
	to_restore_if_fail.emplace_back(tb_chckf, TXT);
	to_delete.push_back(get_old_file(tb_chckf) + TXT);

	// Calculate crcs for table_name.csv into table_name_chck.txt
	try {
		calculate_and_save_crcs_for(tbf, CSV);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	//Rename tables_info.csv to tables_info_old.csv
	std::string tbi_f = get_tables_info_file();
	try { 
		rename_to_old(tbi_f, CSV);
	}
	catch (std::exception& e) {
		restore_from_old(tbi_f, CSV);
		restore_files(to_restore_if_fail);
		throw;
	}
	to_restore_if_fail.emplace_back(tbi_f, CSV);
	to_delete.push_back(get_old_file(tbi_f) + CSV);

	// Remove row for this table from tables_info.csv
	try {
		copy_tbi_rows_except(table_name);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	//Add updated table information to tables_info.csv
	std::vector<std::string> tbi_rows;
	tbi_rows.push_back(tb_info);
	try {
		add_new_rows(tbi_f + CSV, tbi_rows);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	// Backup tables_info_chck.txt
	std::string tbi_chckf = get_chck_file(tbi_f);
	try {
		backup_file(tbi_chckf, TXT);
	}
	catch (std::exception& e) {
		restore_from_old(tbi_chckf, TXT);
		restore_files(to_restore_if_fail);
		throw;
	}
	to_restore_if_fail.emplace_back(tbi_chckf, TXT);
	to_delete.push_back(get_old_file(tbi_chckf) + TXT);

	// Calculate crcs for table_name.csv into table_name_chck.txt
	try {
		calculate_and_save_crcs_for(tbi_f, CSV);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	delete_files(to_delete);
	return rows_added;
}

void disk_storage::get_rows_from_to(const std::string& table_name, std::vector<std::string>& res, const size_t from, const size_t to) {
	std::string filename = DB_PATH + table_name + CSV;
	std::ifstream fin(filename);
	if (!fin.good()) {
		fin.close();
		throw std::ios_base::failure("Couldn't open file " + filename + " for reading.");
	}

	std::string line_buf = "";
	for (size_t i = 1; i <= to; i++) {
		if (!fin.good() || !std::getline(fin, line_buf) || line_buf.empty()) {
			fin.close();
			throw std::logic_error("Couldn't read row #" + std::to_string(i) + " from " + filename);
		}
		if (i >= from) {
			res.push_back(line_buf);
		}
		line_buf.clear();
	}
	fin.close();
}

void disk_storage::get_query_sorting_res_from_to(std::vector<std::string>& res, const size_t from, const size_t to, const bool delete_query, const size_t portions) {
	get_rows_from_to(get_query_sorted_file(), res, from, to);
	if (delete_query) {
		for (size_t i = 1; i <= portions; i++) {
			std::remove((get_portion_file(i) + CSV).data());
		}
		std::remove((get_query_sorted_file_full() + CSV).data());
	}
}

void disk_storage::close_all(std::unordered_map<size_t, std::ifstream>& files) {
	for (auto& p : files) {
		p.second.close();
	}
}

void disk_storage::save_sorting_portion(const size_t portion_number, const std::vector<std::string>& rows) {
	std::string filename = get_portion_file(portion_number) + CSV;
	
	create_if_not_exists(filename);
	std::ofstream fout(filename, std::ios::trunc);
	size_t size = rows.size();
	for (size_t i = 0; i < size; i++) {
		if (!fout.good()) {
			fout.close();
			throw std::ios_base::failure("Cannot write to file " + filename);
		}
		fout << rows[i] << std::endl;
	}
	fout.close();
}

void disk_storage::sort_portions(const size_t portions_count, const std::vector<size_t>& keys, const std::vector<col_info>& table_cols) {
	std::vector<std::string> filenames;
	for (size_t p = 1; p <= portions_count; p++) {
		filenames.push_back(get_portion_file(p) + CSV);
	}

	std::unordered_map<size_t, std::ifstream> files;
	size_t file_id = 1;
	for (auto& fn : filenames) {
		if (is_empty_file(fn)) {
			continue;
		}
		files.emplace(file_id++, std::ifstream(fn));
	}

	rows_binary_heap h(keys);
	std::vector<size_t> keys_to_remove;

	//Read 1 line of each file and push in heap
	for (auto& p : files) {
		std::string line = "";
		if (!p.second.good() || !std::getline(p.second, line) || line.empty()) {
			close_all(files);
			throw std::ios_base::failure("Error reading from temp file.");
		}
		
		row r = csv_parser::row_from_csv(line, table_cols);
		r.push_back((int)p.first);
		h.insert(r);
		if (p.second.eof()) {
			keys_to_remove.push_back(p.first);
		}
	}

	//If any file has no more lines, close it and remove it from the heap
	for (int id : keys_to_remove) {
		files.at(id).close();
		files.erase(id);
	}

	std::string filename = get_query_sorted_file_full() + CSV;
	create_if_not_exists(filename);
	std::ofstream fout(filename, std::ios::trunc);

	std::string buf = "";
	while (!files.empty()) {
		if (buf.length() >= BUF_FLUSH_THRESHOLD) {
			fout << buf;
			buf.clear();
		}

		if (h.size() < files.size()) {
			close_all(files);
			fout.close();
			throw std::logic_error("Error merging temp files.");
		}

		row min = h.extract_min();
		int file_key = std::get<int>(min[min.size() - 1]);
		min.pop_back();
		buf.append(csv_parser::row_to_csv(min));
		buf.append("\n");

		//If this file is already closed, do nothing (this was the last row from it)
		if (!files.contains(file_key)) {
			continue;
		}

		auto& fin = files.at(file_key);

		//Read one more line from this file and add to the heap with the file id
		std::string line = "";
		if (!std::getline(files.at(file_key), line)) {
			//Read error
			if (fin.bad()) {
				close_all(files);
				fout.close();
				throw std::ios_base::failure("Error reading from temp file.");
			}
			//File ended
			fin.close();
			files.erase(file_key);
			continue;
		}
		row temp = csv_parser::row_from_csv(line, table_cols);
		temp.push_back(file_key);
		h.insert(temp);
	}

	fout << buf;
	fout.close();

}

void disk_storage::prepare_query_file() {
	std::string filename = get_query_res_file_full() + CSV;
	create_if_not_exists(filename);
	std::ofstream fout(filename, std::ios::trunc);
	fout.close();
}

void disk_storage::save_query_res_portion(const std::vector<std::string>& csv_rows) {
	std::string filename = get_query_res_file_full() + CSV;
	if (!file_exists(filename)) {
		throw std::logic_error("Query res file does not exist!");
	}
	std::ofstream fout(filename, std::ios::app);
	for (auto& r : csv_rows) {
		fout << r << std::endl;
	}
	fout.close();
}

void disk_storage::update_max_col_lengths(std::vector<size_t>& col_sizes, const std::string csv_row, const std::vector<col_info>& cols_info) {
	row curr = csv_parser::row_from_csv(csv_row, cols_info);
	size_t size = curr.size();
	if (size != col_sizes.size()) {
		throw std::invalid_argument("Invalid column size " + std::to_string(size) + " of row:\n"
			+ csv_row + "\nExpected: " + std::to_string(col_sizes.size()));
	}

	for (size_t i = 0; i < size; i++) {
		std::string field = value_parser::to_string(curr[i]);
		if (field.length() > col_sizes[i]) {
			col_sizes[i] = field.length();
		}
	}
}

std::vector<size_t> disk_storage::get_query_res_max_col_lengths(const size_t col_count, const size_t row_count, const std::vector<col_info>& cols_info) {
	std::string filename = get_query_res_file_full() + CSV;
	if (!file_exists(filename)) {
		throw std::logic_error("Query result file doesn't exist!");
	}

	std::vector<size_t> col_sizes(col_count, 0);

	std::ifstream fin(filename);
	for (size_t i = 1; i <= row_count; i++) {
		std::string line = "";
		if (!getline(fin, line) || line.empty()) {
			fin.close();
			throw std::ios_base::failure("Error reading row #" + std::to_string(i) + " from query result file.");
		}
		try {
			update_max_col_lengths(col_sizes, line, cols_info);
		} catch(std::exception& e) {
			fin.close();
			throw;
		}
	}
	fin.close();
	return col_sizes;
}

void disk_storage::get_query_res_from_to(std::vector<std::string>& res, const size_t from, const size_t to) {
	get_rows_from_to(get_query_res_file(), res, from, to);
}

int disk_storage::space_on_disk(const std::string& table_name) {
	std::string filename = DB_PATH + table_name + CSV;
	std::uintmax_t size_bytes = std::filesystem::file_size(filename);
	int size_kb = size_bytes / KB_SIZE;
	return size_kb + 1;
}

size_t disk_storage::remove_duplicates_in_query_res() {
	std::string act = get_query_res_file_full() + CSV;
	std::string old = get_old_file(get_query_res_file_full()) + CSV;
	rename_to_old(get_query_res_file_full(), CSV);

	std::unordered_set<size_t> cache_set;
	cache_set.reserve(MAX_CACHE_SIZE);
	std::hash<std::string> hasher;

	std::ifstream fin(old);
	if (fin.bad()) {
		fin.close();
		throw std::ios_base::failure("Error opening file " + old + " for reading.");
	}
	create_if_not_exists(act);
	std::ofstream fout(act, std::ios::trunc);

	std::string seen = get_seen_file() + DAT;
	create_if_not_exists(seen);
	std::fstream disk_set(seen, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
	
	size_t unique_lines_count = 0;
	std::string line = "";
	while (std::getline(fin, line)) {
		size_t h = hasher(line);
		
		bool was_in_cache;
		bool just_written = false;
		if (cache_set.size() == MAX_CACHE_SIZE) {
			was_in_cache = cache_set.contains(h);
		}
		else {
			just_written = cache_set.insert(h).second;
			was_in_cache = !just_written;
		}

		if (was_in_cache) {
			continue;
		}

		disk_set.clear();
		disk_set.seekg(0);
		size_t curr_hash;
		bool found = false;
		while (disk_set.read(reinterpret_cast<char*>(&curr_hash), sizeof(size_t))) {
			if (curr_hash == h) {
				found = true;
				break;
			}
		}
		if (found) {
			continue;
		}
		fout << line << std::endl;
		unique_lines_count++;
		if (!just_written) {
			disk_set.write(reinterpret_cast<char*>(&h), sizeof(size_t));
		}
	}

	fin.close();
	fout.close();
	disk_set.close();
	std::remove(seen.data());
	std::remove(old.data());
	return unique_lines_count;
}

#include <cassert>
size_t disk_storage::delete_where(const std::vector<col_info>& cols_info, const formula* f, table_info& ti) {
	std::string table_name = ti.table_name;

	std::vector<std::pair<std::string, std::string>> to_restore_if_fail;
	std::vector<std::string> to_delete;

	//Rename table_name.csv to table_name_old.csv
	std::string tbf = DB_PATH + table_name; //TODO: tova trqbva da e funkciq
	try {
		rename_to_old(tbf, CSV);
	}
	catch (std::exception& e) {
		if (file_exists(get_old_file(tbf) + CSV)) {
			restore_from_old(tbf, CSV);
		}
		throw;
	}
	to_restore_if_fail.emplace_back(tbf, CSV);
	to_delete.push_back(get_old_file(tbf) + CSV);

	//Create new table_name.csv 
	try {
		create_if_not_exists(tbf + CSV);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	//Copy rows which don't satisfy the formula to the new table_name.csv
	size_t deleted_rows = 0;
	try {
		deleted_rows = save_rows_from_backup_where_not(tbf + CSV, get_old_file(tbf) + CSV, cols_info, f);
	}
	catch (std::exception& e) {
		restore_from_old(tbf, CSV);
		throw;
	}

	//Rename table crc file to old
	std::string tb_chck_f = get_chck_file(table_name);
	try {
		rename_to_old(tb_chck_f, TXT);
	} catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		if (file_exists(get_old_file(tb_chck_f) + TXT)) {
			restore_from_old(tb_chck_f, TXT);
		}
		throw;
	}
	to_restore_if_fail.emplace_back(tb_chck_f, TXT);
	to_delete.push_back(get_old_file(tb_chck_f) + TXT);

	//Recalculate crc for table
	try {
		calculate_and_save_crcs_for(tbf, CSV);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	//Rename table_info.csv to old
	std::string tbi_f = get_tables_info_file();
	try {
		rename_to_old(tbi_f, CSV);
	}
	catch (std::exception& e) {
		if (file_exists(get_old_file(tbi_f) + CSV)) {
			restore_from_old(tbi_f, CSV);
		}
		restore_files(to_restore_if_fail);
		throw;
	}
	to_restore_if_fail.emplace_back(tbi_f, CSV);
	to_delete.push_back(get_old_file(tbi_f) + CSV);

	//Save table_info.csv without this table
	try {
		copy_tbi_rows_except(table_name);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	//Append updated table info to table_info.csv with fewer rows
	size_t old_row_count = ti.row_count;
	ti.row_count -= deleted_rows;
	std::vector<std::string> tbi_rows;
	tbi_rows.push_back(csv_parser::tb_info_to_csv(ti));
	try {
		add_new_rows(tbi_f + CSV, tbi_rows);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	//Rename table_info_chck.txt to table_info_chck_old.txt
	std::string tbi_chck_f = get_chck_file(tbi_f);
	try {
		rename_to_old(tbi_chck_f, TXT);
	}
	catch (std::exception& e) {
		if (file_exists(get_old_file(tbi_chck_f) + TXT)) {
			restore_from_old(tbi_chck_f, TXT);
		}
		restore_files(to_restore_if_fail);
		throw;
	}
	to_restore_if_fail.emplace_back(tbi_chck_f, TXT);
	to_delete.push_back(get_old_file(tbi_chck_f) + TXT);

	//Calculate and save crcs to it
	try {
		calculate_and_save_crcs_for(tbi_f, CSV);
	}
	catch (std::exception& e) {
		restore_files(to_restore_if_fail);
		throw;
	}

	delete_files(to_delete);
	return deleted_rows;
}

#include "formula_evaluator.h"
size_t disk_storage::save_rows_from_backup_where_not(const std::string& table_filename, const std::string& old_table_filename, const std::vector<col_info>& cols_info, const formula* f) {
	std::ifstream fin(old_table_filename);
	if (fin.bad()) {
		fin.close();
		throw std::ios_base::failure("Error opening " + table_filename + " for reading.");
	}
	std::ofstream fout(table_filename, std::ios::trunc);

	std::string line = "";
	size_t deleted_rows = 0;
	while (std::getline(fin, line)) {
		if (fin.bad()) {
			fin.close();
			fout.close();
			throw std::ios_base::failure("Error reading from " + table_filename);
		}
		
		row curr = csv_parser::row_from_csv(line, cols_info);
		if (!formula_evaluator::eval(f, curr)) {
			fout << line << std::endl;
		}
		else {
			deleted_rows++;
		}
		line.clear();
	}

	if (fin.bad()) {
		fin.close();
		fout.close();
		throw std::ios_base::failure("Error reading from " + table_filename);
	}

	fin.close();
	fout.close();
	return deleted_rows;
}