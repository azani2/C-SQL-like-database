#include "catch_amalgamated.hpp"
#include "create_sql_parser.h"
#include "col_info_io.h"
#include <stdexcept>

// Error exits:
// 9 invalid_exception throws with a total of 13 different exits:
// table_name_from_line -> 2 : Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.
// schema_colon_index -> 3 : Column description is not in the right format!
//								-- missing column name before :
//								-- missing :
//								-- : at end of schema
// col_type_from_schema -> 1 : Column description is not in the right format!
// col_defaul_value_from_schema -> 1 : Default value string is empty!
// check_no_duplicate_columns -> 1: Table cannot have 2 columns with the same name!
// idx_col_name_from_line -> 1: No index column was specified after Index ON clause.
// extract_schema -> 3 : Invalid table schema! Must be enclosed by ( and ).
//								-- missing (
//								-- missing )
//								-- ) is before (
// tb_from_line -> 3 : Couldn't match index column name idx_col_name to any column name!
//						Index column must be of type Int!
//						Index column cannot have default value!
// has_index_from_line -> 1 : Unexpected trailing text after columns description: ...
//

//----------------------------Error exits:

TEST_CASE("tb_from_line throws when schema is missing") {
	std::string s = "TableName ";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
}

TEST_CASE("tb_from_line throws when there is \"junk\" text in create query") {
	std::string s = "SomeTableName ahaihal(field1:String DEFAULT \"Hellooo, how \"are\" you?\", field2:Int DEFAULT 14, field3:Date DEFAULT 1949-10-27, field4:Double DEFAULT 89.7, field5:DateTime DEFAULT 2035-05-13 07:38:43)asafadfdaf Index ON field1";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
}

TEST_CASE("tb_from_line throws when table name is empty") {
	std::string s = "   (some:Int DEFAULT 0) Index ON some";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
	s = "(textField:String DEFAULT Hello) Index ON textField";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
}

TEST_CASE("tb_from_line throws when a column name is missing") {
	std::string s = "SomeTableName1 (   :Int )";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Column description is not in the right format!");
	s = "SomeOtherTableName2 (:Int )";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Column description is not in the right format!");
}

TEST_CASE("tb_from_line throws when a : is missing") {
	std::string s = "SomeTableName (field1String)";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Column description is not in the right format!");
}

TEST_CASE("tb_from_line throws when a column type is missing") {
	std::string s = "SomeTableName (field1: )";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Column description is not in the right format!");
	s = "SomeTableName (field1:)";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Column description is not in the right format!");
}

TEST_CASE("tb_from_line throws when there are duplicate column names") {
	std::string s = "SomeTableName (field1:Int, field1:Int)";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Table cannot have 2 columns with the same name: field1");
	s = "SomeTableName (something:Int, something:Int, field2:String)";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Table cannot have 2 columns with the same name: something");
	s = "SomeTableName (field11:Int, field2:Int, field11:String)";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Table cannot have 2 columns with the same name: field11");
	s = "SomeTableName (field2:Int, field1:Int, field2:String)";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Table cannot have 2 columns with the same name: field2");
}

TEST_CASE("tb_from_line throws when default value is not specified after DEFAULT clause") {
	std::string s = "SomeTableName (field2:String DEFAULT )";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Default value string is empty!");
	s = "SomeTableName (field1:Int, field2:String DEFAULT , field3:Date)";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Default value string is empty!");
}

TEST_CASE("tb_from_line throws when no index column name was specified after Index ON clause") {
	std::string s = "SomeTableName (field2:String DEFAULT Hello) Index ON";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "No index column was specified after Index ON clause.");
	s = "SomeTableName (field2:String) Index ON";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "No index column was specified after Index ON clause.");
	s = "SomeTableName (field2:String) Index ON   ";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "No index column was specified after Index ON clause.");
}

TEST_CASE("tb_from_line throws when table schema is not full") {
	std::string s = "SomeTableName )(";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
	s = "SomeTableName )something:something(";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
	s = "SomeTableName (filed1:Int";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
	s = "SomeTableName (filed1:Int Index ON field1";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
	s = "SomeTableName filed1:Int)";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Invalid table schema! Must be enclosed by ( and ) and table name cannot be empty.");
}

TEST_CASE("tb_from_line throws when specfied index column doesn't match any column name") {
	std::string s = "SomeTableName (field2:String) Index ON field1";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Couldn't match index column name field1 to any column name!");
	s = "SomeTableName (field1:Int DEFAULT 1, field2:String) Index ON wrongColumn";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Couldn't match index column name wrongColumn to any column name!");
}

TEST_CASE("tb_from_line throws when index column if not of type int") {
	std::string s = "SomeTableName (field1:String, field2:Int, field3:Date, field4:Double, field5:DateTime) Index ON field1";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Index column must be of type Int!");
	s[s.length() - 1] = '3';
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Index column must be of type Int!");
	s[s.length() - 1] = '4';
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Index column must be of type Int!");
	s[s.length() - 1] = '5';
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Index column must be of type Int!");
}

TEST_CASE("tb_from_line throws when index column has default value") {
	std::string s = "SomeTableName (field1:String, field2:Int DEFAULT 12, field3:Date, field4:Double, field5:DateTime) Index ON field2";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Index column cannot have default value!");
}

TEST_CASE("tb_from_line throws when there is treailing text after columns descriptions instead of Index ON or nothing") {
	std::string s = " Birds (Name:String, ID:Int, Habitat:String DEFAULT Everywhere\\, where they live) INDEX ON ID";
	REQUIRE_THROWS_WITH(create_sql_parser::tb_from_line(s), "Unexpected trailing text after columns description: INDEX ON ID");
}

//----------------------------OK exits:


void compare_table(const table& t, const table_info& tbi, const std::vector<col_info>& colsi) {
	REQUIRE(t.info == tbi);
	REQUIRE(t.cols_info.size() == colsi.size());
	for (size_t i = 0; i < colsi.size(); i++) {
		REQUIRE(t.cols_info[i] == colsi[i]);
	}
}

TEST_CASE("tb_from_line correctly parses table with nonsense name") {
	std::string s = "Fjf=/'*(*i-*(-89 (field2:String)";
	table_info tbi{ "Fjf=/'*(*i-*(-89", 1, 0, false, 0 , 0 };
	col_info coli{ "field2", STRING, false, "", false, 0 };
	std::vector<col_info> colsi = { coli };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}

TEST_CASE("tb_from_line correctly parses simple 1 column table with column of any type") {
	std::string s = "SomeTableName (field2:String)";
	table_info tbi{ "SomeTableName", 1, 0, false, 0 , 0};
	col_info coli{ "field2", STRING, false, "", false, 0 };
	std::vector<col_info> colsi = { coli };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);

	s = "TableName2 (field2:Int)";
	tbi.table_name = "TableName2";
	coli.type = INT;
	coli.default_value = 0;
	colsi.pop_back();
	colsi.push_back(coli);
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);

	s = "TableName3 (field2:Double)";
	tbi.table_name = "TableName3";
	coli.type = DOUBLE;
	coli.default_value = 0.0;
	colsi.pop_back();
	colsi.push_back(coli);
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);

	s = "TableName4 (field2:Date)";
	tbi.table_name = "TableName4";
	coli.type = DATE;
	coli.default_value = date{ 1000, 1, 1 };
	colsi.pop_back();
	colsi.push_back(coli);
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);

	s = "TableName5 (field2:DateTime)";
	tbi.table_name = "TableName5";
	coli.type = DATETIME;
	coli.default_value = datetime{ date{ 1000, 1, 1 } ,0 ,0 ,0 };
	colsi.pop_back();
	colsi.push_back(coli);
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}

TEST_CASE("tb_from_line correctly parses simple multicolumn tables with different types") {
	std::string s = "SomeTableName (field1:String, field2:Int, field3:Date, field4:Double, field5:DateTime)";
	table_info tbi{ "SomeTableName", 5, 0, false, 0 , 0};
	col_info coli1{ "field1", STRING, false, "", false, 0 };
	col_info coli2{ "field2", INT, false, 0, false, 1 };
	col_info coli3{ "field3", DATE, false, date{ 1000, 1, 1 }, false, 2};
	col_info coli4{ "field4", DOUBLE, false, 0.0, false, 3 };
	col_info coli5{ "field5", DATETIME, false, datetime{date{ 1000, 1, 1 }, 0, 0, 0 }, false, 4 };
	std::vector<col_info> colsi = { coli1 , coli2, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);

	s = "SomeTableName (field1:String, field2:Int, field21:String, field3:Date, field4:Double, field5:DateTime)";
	col_info coli21{ "field21", STRING, false, "", false, 2 };
	coli3.col_index = 3;
	coli4.col_index = 4;
	coli5.col_index = 5;
	tbi.col_count = 6;
	colsi.clear();
	colsi = { coli1 , coli2, coli21, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}

TEST_CASE("tb_from_line correctly parses simple multicolumn tables with different types with extra spaces") {
	std::string s = "SomeTableName (  field1:String,  field2:Int  , field3:Date  , field4:Double , field5:DateTime)";
	table_info tbi{ "SomeTableName", 5, 0, false, 0 , 0};
	col_info coli1{ "field1", STRING, false, "", false, 0 };
	col_info coli2{ "field2", INT, false, 0, false, 1 };
	col_info coli3{ "field3", DATE, false, date{ 1000, 1, 1 }, false, 2 };
	col_info coli4{ "field4", DOUBLE, false, 0.0, false, 3 };
	col_info coli5{ "field5", DATETIME, false, datetime{date{ 1000, 1, 1 }, 0, 0, 0 }, false, 4 };
	std::vector<col_info> colsi = { coli1 , coli2, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);

	s = "SomeTableName (field1:String   , field2:Int, field21:String    , field3:Date, field4:Double  ,       field5:DateTime   )";
	col_info coli21{ "field21", STRING, false, "", false, 2 };
	coli3.col_index = 3;
	coli4.col_index = 4;
	coli5.col_index = 5;
	tbi.col_count = 6;
	colsi.clear();
	colsi = { coli1 , coli2, coli21, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}
/*
TEST_CASE("tb_from_line correctly parses simple multicolumn tables with different types with no spaces") {
	std::string s = "SomeTableName (field1:String,field2:Int,field3:Date,field4:Double,field5:DateTime)";
	table_info tbi{ "SomeTableName", 5, 0, false, 0, 0 };
	col_info coli1{ "field1", STRING, false, "", false, 0 };
	col_info coli2{ "field2", INT, false, 0, false, 1 };
	col_info coli3{ "field3", DATE, false, date{ 0, 1, 1 }, false, 2 };
	col_info coli4{ "field4", DOUBLE, false, 0.0, false, 3 };
	col_info coli5{ "field5", DATETIME, false, datetime{date{ 0, 1, 1 }, 0, 0, 0 }, false, 4 };
	std::vector<col_info> colsi = { coli1 , coli2, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);

	s = "SomeTableName (field1:String,field2:Int,field21:String,field3:Date,field4:Double,field5:DateTime)";
	col_info coli21{ "field21", STRING, false, "", false, 2 };
	coli3.col_index = 3;
	coli4.col_index = 4;
	coli5.col_index = 5;
	tbi.col_count = 6;
	colsi.clear();
	colsi = { coli1 , coli2, coli21, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}

TEST_CASE("tb_from_line correctly parses multicolumn indexed tables") {
	std::string s = "SomeTableName (field1:String, field2:Int, field3:Date, field4:Double, field5:DateTime) Index ON field2";
	table_info tbi{ "SomeTableName", 5, 0, true, 1, 0 };
	col_info coli1{ "field1", STRING, false, "", false, 0 };
	col_info coli2{ "field2", INT, false, 0, true, 1 };
	col_info coli3{ "field3", DATE, false, date{ 0, 1, 1 }, false, 2 };
	col_info coli4{ "field4", DOUBLE, false, 0.0, false, 3 };
	col_info coli5{ "field5", DATETIME, false, datetime{date{ 0, 1, 1 }, 0, 0, 0 }, false, 4 };
	std::vector<col_info> colsi = { coli1 , coli2, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);

	s = "SomeTableName (field1:String, field2:Int, field3:DateTime, field4:Double, field5:String, field6:Int) Index ON field6";
	tbi.col_count = 6;
	col_info coli21{ "field21", STRING, false, "", false, 2 };
	coli2.is_indexed = false;
	tbi.index_column_index = 5;
	col_info coli6{ "field6", INT, false, 0, true, 5 };

	coli3.type = DATETIME;
	coli3.default_value = datetime{ date{ 0, 1, 1 }, 0, 0, 0 };
	coli5.type = STRING;
	coli5.default_value = "";

	colsi.clear();
	colsi = { coli1 , coli2, coli3, coli4, coli5, coli6 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}
*/
TEST_CASE("tb_from_line correctly parses multicolumn tables with different types with default values") {
	std::string s = "SomeTableName (field1:String DEFAULT \"Hellooo\\, how \"are\" you?\", field2:Int DEFAULT 14, field3:Date DEFAULT 1949-10-27, field4:Double DEFAULT 89.7, field5:DateTime DEFAULT 2035-05-13 07:38:43)";
	table_info tbi{ "SomeTableName", 5, 0, false, 0, 0 };
	col_info coli1{ "field1", STRING, true, "\"Hellooo, how \"are\" you?\"", false, 0 };
	col_info coli2{ "field2", INT, true, 14, false, 1 };
	col_info coli3{ "field3", DATE, true, date{ 1949, 10, 27 }, false, 2 };
	col_info coli4{ "field4", DOUBLE, true, 89.7, false, 3 };
	col_info coli5{ "field5", DATETIME, true, datetime{ date{ 2035, 5, 13 }, 7, 38, 43 }, false, 4 };
	std::vector<col_info> colsi = { coli1 , coli2, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);

	s = "SomeTableName (field1:String DEFAULT \"Hellooo\\, how \"are\" you?\", field2:Int DEFAULT 14, field21:String DEFAULT \"Some text\\, that contains \"quotes\" haha\"  , field3:Date DEFAULT 1949-10-27, field4:Double DEFAULT 89.7, field5:DateTime DEFAULT 2035-05-13 07:38:43)";
	col_info coli21{ "field21", STRING, true, "\"Some text\, that contains \"quotes\" haha\"", false, 2 };
	coli3.col_index = 3;
	coli4.col_index = 4;
	coli5.col_index = 5;
	tbi.col_count = 6;
	colsi.clear();
	colsi = { coli1 , coli2, coli21, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}

TEST_CASE("tb_from_line correctly parses multicolumn tables with different types with indexed column") {
	std::string s = "SomeTableName (field1:String, field2:Int, field3:Date, field4:Double, field5:DateTime) Index ON field2";
	table_info tbi{ "SomeTableName", 5, 0, true, 1, 0 };
	col_info coli1{ "field1", STRING, false, "", false, 0 };
	col_info coli2{ "field2", INT, false, 0, true, 1 };
	col_info coli3{ "field3", DATE, false, date{ 1000, 1, 1 }, false, 2 };
	col_info coli4{ "field4", DOUBLE, false, 0.0, false, 3 };
	col_info coli5{ "field5", DATETIME, false, datetime{date{ 1000, 1, 1 }, 0, 0, 0 }, false, 4 };
	std::vector<col_info> colsi = { coli1 , coli2, coli3, coli4, coli5 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}

TEST_CASE("tb_from_line correctly parses multicolumn tables with different types with default values and an indexed column") {
	std::string s = "SomeTableName (field1:String DEFAULT \"Hellooo\\, how \"are\" you?\", field2:Int, field21:String DEFAULT \"Some text\\, that contains \"quotes\" haha\"  , field3:Date DEFAULT 1949-10-27, field4:Double DEFAULT 89.7, field5:DateTime DEFAULT 2035-05-13 07:38:43) Index ON field2";
	table_info tbi{ "SomeTableName", 6, 0, true, 1, 0 };
	col_info coli1{ "field1", STRING, true, "\"Hellooo, how \"are\" you?\"", false, 0 };
	col_info coli2{ "field2", INT, false, 0, true, 1 };
	col_info coli3{ "field21", STRING, true, "\"Some text, that contains \"quotes\" haha\"", false, 2 };
	col_info coli4{ "field3", DATE, true, date{ 1949, 10, 27 }, false, 3 };
	col_info coli5{ "field4", DOUBLE, true, 89.7, false, 4 };
	col_info coli6{ "field5", DATETIME, true, datetime{ date{ 2035, 5, 13 }, 7, 38, 43 }, false, 5 };
	std::vector<col_info> colsi = { coli1 , coli2, coli3, coli4, coli5, coli6 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}

TEST_CASE("tb_from_line correctly parses string default values with commas") {
	std::string s = "SomeTableName (field1:String DEFAULT \"Hellooo\\, how \"are\" you?\", field2:Int, field21:String DEFAULT \"Some text\\, that contains \"quotes\\, commas\\, and more quotes: \" haha\"  , field3:Date DEFAULT 1949-10-27, field4:Double DEFAULT 89.7, field5:DateTime DEFAULT 2035-05-13 07:38:43) Index ON field2";
	table_info tbi{ "SomeTableName", 6, 0, true, 1, 0 };
	col_info coli1{ "field1", STRING, true, "\"Hellooo, how \"are\" you?\"", false, 0 };
	col_info coli2{ "field2", INT, false, 0, true, 1 };
	col_info coli3{ "field21", STRING, true, "\"Some text, that contains \"quotes, commas, and more quotes: \" haha\"", false, 2 };
	col_info coli4{ "field3", DATE, true, date{ 1949, 10, 27 }, false, 3 };
	col_info coli5{ "field4", DOUBLE, true, 89.7, false, 4 };
	col_info coli6{ "field5", DATETIME, true, datetime{ date{ 2035, 5, 13 }, 7, 38, 43 }, false, 5 };
	std::vector<col_info> colsi = { coli1 , coli2, coli3, coli4, coli5, coli6 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}

TEST_CASE("tb_from_line correctly parses string default values with commas unquoted") {
	std::string s = "SomeTableName (field1:String DEFAULT Hellooo\\, how \"are\" you?, field2:Int, field21:String DEFAULT Some text\\, that contains \"quotes\\, commas\\, and more quotes: \" haha  , field3:Date DEFAULT 1949-10-27, field4:Double DEFAULT 89.7, field5:DateTime DEFAULT 2035-05-13 07:38:43) Index ON field2";
	table_info tbi{ "SomeTableName", 6, 0, true, 1, 0 };
	col_info coli1{ "field1", STRING, true, "Hellooo, how \"are\" you?", false, 0 };
	col_info coli2{ "field2", INT, false, 0, true, 1 };
	col_info coli3{ "field21", STRING, true, "Some text, that contains \"quotes, commas, and more quotes: \" haha", false, 2 };
	col_info coli4{ "field3", DATE, true, date{ 1949, 10, 27 }, false, 3 };
	col_info coli5{ "field4", DOUBLE, true, 89.7, false, 4 };
	col_info coli6{ "field5", DATETIME, true, datetime{ date{ 2035, 5, 13 }, 7, 38, 43 }, false, 5 };
	std::vector<col_info> colsi = { coli1 , coli2, coli3, coli4, coli5, coli6 };
	compare_table(create_sql_parser::tb_from_line(s), tbi, colsi);
}