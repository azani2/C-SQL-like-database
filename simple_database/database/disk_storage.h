#pragma once
#ifndef DISK_STORAGE_HEADER
#define DISK_STORAGE_HEADER
#include <string>
#include <vector>
#include "table.h"
#include "formula.h"
#include <unordered_map>

class disk_storage {
private:
	static const size_t BUF_FLUSH_THRESHOLD;

	static const int KB_SIZE;
	static const int MAX_CACHE_SIZE;

	static const std::string DB_NAME;
	static const std::string DB_PATH;
	static const std::string QUERY_NAME;
	static const std::string QUERY_PATH;
	static const std::string TABLES_CNT_FILE_NAME;
	static const std::string TABLES_INFO_FILE_NAME;
	static const std::string SEEN_FILE_NAME;
	static const std::string QUERY_SORTED;
	static const std::string QUERY_RES;
	static const std::string QUERY_PORTION_NAME;
	static const std::string CSV;
	static const std::string TXT;
	static const std::string DAT;
	static const std::string CHCK;
	static const std::string COLS;
	static const std::string OLD;
	static const std::string NEW;

	static std::string get_seen_file();
	static std::string get_cols_file(const std::string&);
	static std::string get_chck_file(const std::string&);
	static std::string get_tables_count_file();
	static std::string get_tables_info_file();
	static std::string get_query_sorted_file_full();
	static std::string get_query_sorted_file();
	static std::string get_query_res_file_full();
	static std::string get_query_res_file();
	static std::string get_portion_file(const size_t);
	static std::string get_old_file(const std::string&);

	// Doesn't throw
	static bool file_exists(const std::string&);
	static void close_all(std::unordered_map<size_t, std::ifstream>&);
	// Doesn't throw
	static bool is_empty_file(const std::string&);
	// Throws if created file fstream is not good for writing
	static void create_if_not_exists(const std::string&);
	// Calls std::remove for every filename in the vector
	static void delete_files(const std::vector<std::string>&);
	// Renames file.smth to file_old.smth
	// Creates new empty file.smth
	// Copies content of file_old.smth to file.smth
	// Throws and undoes chnges if file.smth or file_old.smth fails to open
	//		or there is an error reading from file.smth
	// Throws and does nothing if file.smth doesn't exist
	static void backup_file(const std::string&, const std::string&);
	static void delete_safe(const std::string&, const std::string&, const std::vector<std::pair<std::string, std::string>>&);
	// Renames filename.extension to filename_old.extension
	// Extension string must include '.'.
	// Throws if renaming fails.
	static void rename_to_old(const std::string&, const std::string&);
	// If actual file exists, deletes it before renaming old to actual
	static void restore_from_old(const std::string&, const std::string&);
	// Deletes the new actual files if they exist and renames the old ones to actual
	static void restore_files(const std::vector<std::pair<std::string, std::string>>&);

	// Throws if file failed to open
	static void calculate_crcs(const std::string&, std::vector<size_t>&);
	// Creates crc file if there is none
	// If already exists truncates it - shouldn't happen - 
	//		use recalculate_and_save_crcs_for() instead
	// Throws if crc file fails to open
	// Throws if filename is invalid
	// Throws if crc array is empty (shouldn't be if 
	//		calculate_crcs() was just called
	static void save_crcs(const std::string&, const std::vector<size_t>&);
	// Throws if crc file for this file fails to open
	// Throws if chunck count in the beginning of the crc file
	//		doesn't match the number of crc codes recorded in it
	//		(number of lines - 1)
	static void read_crcs(const std::string&, std::vector<size_t>&);
	static void calculate_and_save_crcs_for(const std::string&, const std::string&);
	static void data_integrity_check_file(const std::string&, const std::string&);
	
	// If there is no tables count file creates it and writes 0
	// Throws if tables count file fails to open.
	static size_t read_tables_count();
	// Throws  if tables count file fails to open
	static void save_tables_count(size_t);
	// Reads the current tables count and rewrites it with 1 less
	// Throws if tables count is 0
	// Throws if tables count file fails to open for reading or writing
	static void decrement_tables_count();
	// Reads the current tables count and rewrites it with 1 bigger
	// Throws if tables count file fails to open for reading or writing
	static void increment_tables_count();

	// Throws if table_name_cols.csv doesn't exist
	// Throws if table_name_cols.csv file fails to open
	// Throws if col_count =/= number of lines in the table_name_cols.csv file
	static void read_cols_info(const std::string&, const size_t, std::vector<col_info>&);
	// Throws if tables_info.csv file doesn't exist
	// Throws if tables count passed doesn't match number of lines in tables_info.csv
	// TODO: This function receives tableS_count as an arg from database
	//			- why did i not do this for the rest and skip reading it every time...
	//			- fix them later
	static void read_table_metadata(const size_t, const std::string&, std::vector<col_info>&);

	// Reads current tables count N and then reads N lines from tables_info file
	// Throws if N lines cannot be read before end of file
	// Throws if tables count file fails to open
	// Throws if table_info file fails to open
	static void read_tables_names(std::vector<std::string>&);

	static void save_tb_info(const table_info&);
	static void save_cols_info(const table&);
	
	// Copies all rows of table_info_old.csv to table_info.csv except row of this table
	// Throws if table_info_old.csv doesn't exist
	// Throws if either file fails to open
	// Throws if line count in table_info_old.csv is less than table count recorded in tables_count.txt
	// Throws if getline fails for any line
	// Throws if any line doesn't contain ',' => not corret csv format
	// TODO: check if there is still more tables after tables count recorded
	//		in tables_count.csv of lines has been read => it is inaccurate and
	//		there is actually more tables or other data corruption
	static void copy_tbi_rows_except(const std::string&);

	static size_t add_new_rows(const std::string&, const std::vector<std::string>&);

	static void update_max_col_lengths(std::vector<size_t>&, const std::string, const std::vector<col_info>&);

	static size_t save_rows_from_backup_where_not(const std::string&, const std::string&, const std::vector<col_info>&, const formula*);
	

public:
	disk_storage() = delete;
	disk_storage(const disk_storage&) = delete;

	// Creates tables_info.csv and tables_count.txt if they don't exist
	// And their checksum files
	static void prepare();

	static void data_integrity_check();

	static void read_tables_metadata(std::vector<table>&);

	static void save_table(const table&);

	static void delete_table(const std::string&);

	static int space_on_disk(const std::string&);

	static size_t save_rows(const std::string&, const std::vector<std::string>&, const std::string&);

	// Pushes lines from "from" to "to" from the table_name.csv file into the vector, incusive of "from" and "to" rows
	static void get_rows_from_to(const std::string&, std::vector<std::string>&, const size_t, const size_t);
	
	static void save_sorting_portion(const size_t, const std::vector<std::string>&);
	static void sort_portions(const size_t, const std::vector<size_t>&, const std::vector<col_info>&);
	static void get_query_sorting_res_from_to(std::vector<std::string>&, const size_t, const size_t, const bool delete_query = false, const size_t portions = 0);

	static void prepare_query_file();
	static void save_query_res_portion(const std::vector<std::string>&);
	static void get_query_res_from_to(std::vector<std::string>&, const size_t, const size_t);
	static std::vector<size_t> get_query_res_max_col_lengths(const size_t, const size_t, const std::vector<col_info>&);

	static size_t remove_duplicates_in_query_res();

	static size_t delete_where(const std::vector<col_info>&, const formula*, table_info&);
};

#endif
