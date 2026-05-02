#include "catch_amalgamated.hpp"
#include "insert_sql_parser.h"
#include "value_io.h"
#include "column_types.h"
#include "string_utils.h"
#include "test_utils.h"
#include <random>

TEST_CASE("tb_name_from_line correctly parses table name from insert command") {
	std::string s = " INTO Sample {(1, \"Test\", 1), (2, \"something else\", 100)}";
	std::string expected_name = "Sample";
	REQUIRE(expected_name == insert_sql_parser::tb_name_from_line(s));

	s = "INTO Sample {(1, \"Test\", 1), (2, \"something else\", 100)}";
	REQUIRE(expected_name == insert_sql_parser::tb_name_from_line(s));

	s = " INTO TestName {(1, \"Test\", 1), (2, \"something else\", 100)}";
	expected_name = "TestName";
	REQUIRE(expected_name == insert_sql_parser::tb_name_from_line(s));
}

TEST_CASE("tb_name_from_line throws when command does not contain \"INTO \" statement") {
	std::string s = " Sample {(1, \"Test\", 1), (2, \"something else\", 100)}";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: \"Insert INTO \" must be followed by a valid table name.");
	s = "fakjqnfapfhafna";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: \"Insert INTO \" must be followed by a valid table name.");
	s = "hauf ljailfn douabfl {dawoifw aofhfa}";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: \"Insert INTO \" must be followed by a valid table name.");
	s = "";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: \"Insert INTO \" must be followed by a valid table name.");
}

TEST_CASE("tb_name_from_line throws when command does not contain rows") {
	std::string s = "INTO Sample";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
	s = "INTO Sample  ";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
	s = "INTO     ";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
}

TEST_CASE("tb_name_from_line throws when command contains no table name") {
	std::string s = " INTO  {(1, \"Test\", 1), (2, \"something else\", 100)}";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
	s = " INTO {(1, \"Test\", 1), (2, \"something else\", 100)}";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
	s = "INTO {(1, \"Test\", 1), (2, \"something else\", 100)}";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
}

//-----------------ROWS:

TEST_CASE("rows_from_line correctly parses simple table row") {
	std::string s = "INTO Sample {(1, \"Test\", 1)}";
	table t;
	t.info = table_info{ "Sample",3,0,0,0,0 };
	t.cols_info = { col_info{"ID",INT,false,0,false,0 } ,
		col_info{ "Name",STRING,false,"",false,1 },
		col_info{ "Age", INT,false,0,false,2 } 
	};
	std::vector<row> expected = {
		{1, "\"Test\"", 1}
	};
	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);

	s = "INTO   Sample   {(  1   ,    \"Test\"  ,  1   )}";
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses simple table rows") {
	std::string s = " INTO Sample {(1, \"Test\", 1), (2, \"something else\", 100)}";
	table t;
	t.info = table_info{ "Sample",3,0,0,0,0 };
	t.cols_info = { col_info{"ID",INT,false,0,false,0 } ,
		col_info{ "Name",STRING,false,"",false,1 },
		col_info{ "Age", INT,false,0,false,2 }
	};
	std::vector<row> expected = {
		{1, "\"Test\"", 1},
		{2, "\"something else\"", 100}
	};
	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);

	s = "   INTO Sample   {(1,    \"Test\", 1)   ,     (2, \"something else\"    , 100)}";
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses table row with index") {
	std::string s = " INTO Sample {(\"Test\", 1)}";
	table t;
	t.info = table_info{ "Sample",3,0,true,0,5 };
	t.cols_info = { col_info{"ID",INT,false,0,true,0 } ,
		col_info{ "Name",STRING,false,"",false,1 },
		col_info{ "Age", INT,false,0,false,2 }
	};
	std::vector<row> expected = {
		{6, "\"Test\"", 1}
	};
	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);

	s = "   INTO Sample   {(    \"Test\", 1)   ,     ( \"something else\"    , 100)}";
	expected.push_back({ 7, "\"something else\"", 100 });
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with different types") {
	std::string s = " INTO Sample {(1, \"Test\", 1, 2000-01-01), (2, \"something else\", 100, 2001-02-02)}";
	table t;
	t.info = table_info{ "Sample",4,0,false,0,0 };
	t.cols_info = { col_info{"ID",INT,false,0,false,0 } ,
		col_info{ "Name",STRING,false,"",false,1 },
		col_info{ "Age",INT,false,0,false,2 },
		col_info{ "Date",DATE,false,column_types::db_default_value(DATE),false,3}
	};
	std::vector<row> expected = {
		{1, "\"Test\"", 1, date{2000,1,1}},
		{2, "\"something else\"", 100, date{2001,2,2}}
	};
	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);

	s = "   INTO Sample   {(1,    \"Test\", 1  ,  2000-01-01   )   ,     (2, \"something else\"    , 100,  2001-02-02   )}";
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with default values and missing columns") {
	std::string s = " INTO Sample {(1)}";
	table t;
	t.info = table_info{ "Sample",3,0,false,0,0 };
	t.cols_info = { col_info{"ID",INT,false,0,false,0 } ,
		col_info{ "Floating",DOUBLE,true,3.14,false,1 },
		col_info{ "Date",DATE,true,date{2026,1,20},false,2}
	};
	std::vector<row> expected = {
		{1, 3.14, date{2026,1,20}}
	};
	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with index column in the middle") {
	std::string s = "INTO Sample {(\"Alice\", 25)}";
	table t;
	t.info = table_info{ "Sample",3,0,true,1,10 };
	t.cols_info = { col_info{"Name",STRING,false,0,false,0 } ,
		col_info{ "ID",INT,false,0,true,1 },
		col_info{ "Age",INT,false,0,false,2}
	};
	std::vector<row> expected = {
		{"\"Alice\"",11,25}
	};
	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with specified values for columns with default values") {
	std::string s = "INTO Sample {(\"Alice\", 10, 25)}";
	table t;
	t.info = table_info{ "Sample",3,0,false,0,0 };
	t.cols_info = { col_info{"Name",STRING,false,0,false,0 } ,
		col_info{ "Something",INT,true,100,false,1 },
		col_info{ "Age",INT,true,18,false,2}
	};
	std::vector<row> expected = {
		{"\"Alice\"",10,25}
	};
	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with specified and unspecified defaults") {
	std::string s = "INTO Table123 {(1,2), (1,2,3), (1,,2), (,1,2), (,,1), ()}";
	table t;
	t.info = table_info{"Table123",3,0,false,0,0};
	t.cols_info = {
		col_info{ "Col1",INT,true,111,false,0 },
		col_info{ "Col2",INT,true,222,false,1 },
		col_info{ "Col3",INT,true,333,false,2 }
	};
	std::vector<row>expected = {
		{1,2,333},
		{1,2,3},
		{1,222,2},
		{111,1,2},
		{111,222,1},
		{111,222,333}
	};
	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);

	s = "INTO Table123 {  (1   ,   2)  ,  (   1,  2, 3  ), (1,  ,2), (,  1,2   ), (  , ,1  ), (   )}";
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with specified and unspecified defaults of different types") {
	std::string s = "INTO Sample {(1, 9.9), (2, ,Fantastic!), (3)}";
	table t;
	t.info = table_info{ "Sample",3,0,false,0,0 };
	t.cols_info = {
		col_info{"ID", INT,    false, 0,    false, 0},
		col_info{"Score", DOUBLE, true, 10.0, false, 1},
		col_info{"Comment", STRING, true, "N/A", false, 2}
	};
	std::vector<row> expected = {
		{1,9.9, "N/A"},
		{2,10.0,"Fantastic!"},
		{3,10.0,"N/A"}
	};
	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses datetime with indexed column") {
	std::string s = "INTO Logs {(\"Start\", 2030-12-01 10:15:30)}";
	table t;
	t.info = table_info{ "Logs",3,0,true,0,100 };
	t.cols_info = {
		col_info{"ID", INT, false, 0, true, 0},
		col_info{"EventName", STRING, false, "", false, 1},
		col_info{"Time", DATETIME, false, column_types::db_default_value(DATETIME), false, 2}
	};

	std::vector<row> expected = {
		{101, "\"Start\"", datetime{2030,12,1,10,15,30}}
	};

	std::vector<row> actual; 
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses escaped ',' and ')' in string columns") {
	std::string s = "(INTO Sample {(1, \"hello\\, world (test\\)\", 10)}";
	table t;
	t.info = table_info{ "Sample",3,0,false,0,0 };
	t.cols_info = {
		col_info{"ID", INT, false, 0, false, 0},
		col_info{"Text", STRING, false, "", false, 1},
		col_info{"Value", INT, false, 0, false, 2}
	};

	std::vector<row> expected = {
		{1, "\"hello, world (test)\"", 10}
	};

	std::vector<row> actual;
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses quotes inside string columns") {
	std::string s = "INTO Sample {(1, \"He said \"Hello!\"\", 5)}";
	table t;
	t.info = table_info{ "Sample",3,0,false,0,0 };
	t.cols_info = {
		col_info{"ID", INT, false, 0, false, 0},
		col_info{"Text", STRING, false, "", false, 1},
		col_info{"Value", INT, false, 0, false, 2}
	};

	std::vector<row> expected = {
		{1, "\"He said \"Hello!\"\"", 5}
	};

	std::vector<row> actual; 
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with negative integer and double values") {
	std::string s = "INTO Sample {(-1, -3.14)}";
	table t;
	t.info = table_info{ "Sample",2,0,false,0,0 };
	t.cols_info = {
		col_info{"Delta", INT, false, 0, false, 0},
		col_info{"Score", DOUBLE, false, 0.0, false, 1}
	};

	std::vector<row> expected = {
		{-1, -3.14}
	};

	std::vector<row> actual; 
	insert_sql_parser::rows_from_line(s, t, actual); 
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly applies default value for middle column") {
	std::string s = "INTO Sample {(1,, , 2024-01-01)}";
	table t;
	t.info = table_info{ "Sample",4,0,false,0,0 };
	t.cols_info = {
		col_info{"ID", INT, false, 0, false, 0},
		col_info{"Name", STRING, true, "\"unknown\"", false, 1},
		col_info{"Age", INT, true, 18, false, 2},
		col_info{"Date", DATE, false, column_types::db_default_value(DATE), false, 3}
	};

	std::vector<row> expected = {
		{1, "\"unknown\"",18, date{2024,1,1}}
	};

	std::vector<row> actual; 
	insert_sql_parser::rows_from_line(s, t, actual); 
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with index and default values") {
	std::string s = "INTO Sample {(\"Alice\"), (\"Bob\")}";
	table t;
	t.info = table_info{ "Sample",3,0,true,0,50 };
	t.cols_info = {
		col_info{"ID", INT, false, 0, true, 0},
		col_info{"Name", STRING, false, "", false, 1},
		col_info{"Age", INT, true, 18, false, 2}
	};

	std::vector<row> expected = {
		{51, "\"Alice\"", 18},
		{52, "\"Bob\"", 18}
	};

	std::vector<row> actual; 
	insert_sql_parser::rows_from_line(s, t, actual);
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses many rows") {
	std::string s = "INTO Sample {";
	table t;
	t.info = table_info{ "Sample",3,0,true,0,100 };
	t.cols_info = {
		col_info{"ID", INT, false, 0, true, 0},
		col_info{"Score", DOUBLE, false, "", false, 1},
		col_info{"Birthdate", DATE, false, column_types::db_default_value(DATE), false, 2}
	};
	std::vector<row> expected;
	for (size_t i = 1; i < 100; i++) {
		s.append(", (");
		int id = 100 + i;
		double score = i / 3.14;
		double shortened_score = std::stod(std::to_string(score));
		int year = 2000 + (i % 4);
		unsigned int month = (i % 12) + 1;
		unsigned int day = (i % 28) + 1;
		s.append(std::to_string(score));
		s.append(",");
		s.append(string_utils::date_to_string(date{ year, month, day }));
		s .append( ")");
		row curr;
		curr.push_back(id);
		curr.push_back(shortened_score);
		curr.push_back(date{ year, month, day });
		expected.push_back(curr);
	}
	s.append("}");
	std::vector<row> actual; 
	insert_sql_parser::rows_from_line(s, t, actual); 
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with date datetime and double") {
	std::string s = "INTO Sample {(2020-01-01, 2030-05-10 12:30:45, 99.9)}";
	table t;
	t.info = table_info{ "Sample",3,0,false,0,0 };
	t.cols_info = {
		col_info{"D", DATE, false, column_types::db_default_value(DATE), false, 0},
		col_info{"T", DATETIME, false, column_types::db_default_value(DATETIME), false, 1},
		col_info{"F", DOUBLE, false, 0.0, false, 2}
	};

	std::vector<row> expected = {
		{date{2020,1,1}, datetime{date{2030,5,10},12,30,45}, 99.9}
	};

	std::vector<row> actual; 
	insert_sql_parser::rows_from_line(s, t, actual); 
	compare_rows(expected, actual);
}

TEST_CASE("rows_from_line correctly parses rows with date datetime and double specified and unspecified defaults") {
	std::string s =
		"INTO Sample {"
		"(2020-01-01, 2030-05-10 12:30:45, 99.9), "
		"(2020-01-01, 2030-05-10 12:30:45), "
		"(2020-01-01,,99.9), "
		"(,2030-05-10 12:30:45,99.9), "
		"(,,99.9), "
		"(,,), "
		"(,), "
		"()"
		"}";

	table t;
	t.info = table_info{ "Sample",3,0,false,0,0 };
	t.cols_info = {
		col_info{"D",DATE,true,date{2000,1,1},false,0},
		col_info{"T",DATETIME,true,datetime{ date{1999,12,31}, 23, 59, 59 },false,1},
		col_info{"F",DOUBLE,true,3.14,false,2}
	};

	std::vector<row> expected = {
		{ date{2020,1,1}, datetime{date{2030,5,10},12,30,45}, 99.9 },
		{ date{2020,1,1}, datetime{date{2030,5,10},12,30,45}, 3.14 },
		{ date{2020,1,1}, datetime{date{1999,12,31},23,59,59}, 99.9 },
		{ date{2000,1,1}, datetime{date{2030,5,10},12,30,45}, 99.9 },
		{ date{2000,1,1}, datetime{date{1999,12,31},23,59,59}, 99.9 },
		{ date{2000,1,1}, datetime{date{1999,12,31},23,59,59}, 3.14 },
		{ date{2000,1,1}, datetime{date{1999,12,31},23,59,59}, 3.14 },
		{ date{2000,1,1}, datetime{date{1999,12,31},23,59,59}, 3.14 }
	};

	std::vector<row> actual; 
	insert_sql_parser::rows_from_line(s, t, actual); 
	compare_rows(expected, actual);

	s =
		"INTO Sample {"
		"( 2020-01-01 , 2030-05-10 12:30:45 , 99.9 ),"
		"( 2020-01-01 , 2030-05-10 12:30:45 ),"
		"( 2020-01-01 ,  , 99.9 ),"
		"(  , 2030-05-10 12:30:45 , 99.9 ),"
		"(  ,  , 99.9 ),"
		"(  ,  ,  ),"
		"(,     ), "
		"(     )"
		"}";

	insert_sql_parser::rows_from_line(s, t, actual); 
	compare_rows(expected, actual);
}

/*
TEST_CASE("rows_from_line correctly parses 10 000 random rows") {
	const size_t ROWS = 10'000;
	table t;
	t.info = { "SomeTable",5,0,true,0,1000 };
	t.cols_info = {
		col_info{"ID",INT,false,0,true,0 },
		col_info{ "Name",STRING,true,"default",false,1 },
		col_info{ "Score",DOUBLE,true,10.0,false,2 },
		col_info{ "Birthdate",DATE,true,date{2000,1,1},false,3 },
		col_info{ "AccountCreationTime",DATETIME,true,datetime{date{2000,12,31},0,0,0},false,4 }
	};
	std::vector<row> expected;
	std::string s = "";
	std::vector<row> actual;

	auto start = std::chrono::steady_clock::now();

	generate_test_data(s, t, ROWS, expected);

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> dur = end - start;
	std::cout << "\nTime for " << ROWS << " rows inserted (slower version): " << std::endl;
	std::cout << "Generate test data: " << std::to_string(dur.count()) << std::endl;

	start = std::chrono::steady_clock::now();
	insert_sql_parser::rows_from_line(s, t, actual);
	end = std::chrono::steady_clock::now();
	dur = end - start;
	std::cout << "rows_from_line: " << std::to_string(dur.count()) << std::endl;
	compare_rows(expected, actual);
}
/* Takes some 2 minutes
TEST_CASE("rows_from_line correctly parses 100 000 random rows") {
	const size_t ROWS = 100'000;
	table t;
	t.info = { "SomeTable",5,0,true,0,1000 };
	t.cols_info = {
		col_info{"ID",INT,false,0,true,0 },
		col_info{ "Name",STRING,true,"default",false,1 },
		col_info{ "Score",DOUBLE,true,10.0,false,2 },
		col_info{ "Birthdate",DATE,true,date{2000,1,1},false,3 },
		col_info{ "AccountCreationTime",DATETIME,true,datetime{date{2000,12,31},0,0,0},false,4 }
	};
	std::vector<row> actual;
	std::vector<row> expected;
	std::string s = "";

	auto start = std::chrono::steady_clock::now();

	generate_test_data(s, t, ROWS, expected);

	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> dur = end - start;
	std::cout << "\nTime for " << ROWS << " rows inserted (slower version): " << std::endl;
	std::cout << "Generate test data: " << std::to_string(dur.count()) << std::endl;

	start = std::chrono::steady_clock::now();
	insert_sql_parser::rows_from_line(s, t, actual);
	end = std::chrono::steady_clock::now();
	dur = end - start;
	std::cout << "rows_from_line: " << std::to_string(dur.count()) << std::endl;
	compare_rows(expected, actual);
}
*/

//---------------------------ERROR EXITS:------------------------

// Total throws: 
// 
// tb_name_from_line: 4 :
//						- Missing "INTO " statement
//						- Missing start of rows " {"
//						- Name is empty
//						- Name contains whitespace
// rows_from_line: 5 :
//			get_rows: 4 : 
//						- Missing start or rows " {"
//						- Missing end of rows "}"
//						- Empty rows "{}"
//						- extract_next_row returns <= 1 
//			extract_next_row: 1 :
//						- Missing closing bracket
//						- Row has fewer columns than the table
//

TEST_CASE("tb_name_from_line throws if \"INTO \" is missing") {
	std::string s = "Sample {(1,2,3)}";

	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: \"Insert INTO \" must be followed by a valid table name.");
}

TEST_CASE("tb_name_from_line throws if \" {\" is missing") {
	std::string s = "INTO Sample (1,2,3)}";

	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
}

TEST_CASE("tb_name_from_line throws if table name is empty") {
	std::string s = "INTO  {(1,2,3)}";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
	
	s = "INTO {(1,2,3)}";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
}

TEST_CASE("tb_name_from_line throws if table name contains spaces") {
	std::string s = "INTO Some Table Name {(1,2,3)}";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot contain spaces!");

	s = "INTO    {(1,2,3)}";
	REQUIRE_THROWS_WITH(insert_sql_parser::tb_name_from_line(s), "Invalid insert query: Table name cannot contain spaces!");
}

TEST_CASE("rows_from_line throws when column record count doesn't match the table's expected column count") {
	std::string s = "INTO Sample {(1, \"Test\")}";
	table t;
	t.info = table_info{ "Sample",3,0,0,0,0 };
	t.cols_info = { col_info{"ID",INT,false,0,false,0 } ,
		col_info{ "Name",STRING,false,"",false,1 },
		col_info{ "Age", INT,false,0,false,2 }
	};
	std::vector<row> actual;
	REQUIRE_THROWS_WITH(insert_sql_parser::rows_from_line(s, t, actual), "Row (1, \"Test\") has invalid number of columns: 2, expected: 3.");
}

TEST_CASE("rows_from_line throws if \" {\" is missing") {
	std::string s = "INTO Sample (1,2,3)}";
	table t = simple_table();
	std::vector<row> v;


	REQUIRE_THROWS_WITH(insert_sql_parser::rows_from_line(s, t, v), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
}

TEST_CASE("rows_from_line throws if \"}\" is missing") {
	std::string s = "INTO Sample {(1,2,3)";
	table t = simple_table();
	std::vector<row> v;


	REQUIRE_THROWS_WITH(insert_sql_parser::rows_from_line(s, t, v), "Invalid insert query: Table name cannot be empty and must be followed by \" {rows to be inserted}\".");
}

TEST_CASE("rows_from_line throws if rows are empty \"{}\"") {
	std::string s = "INTO Sample {}";
	table t = simple_table();
	std::vector<row> v;


	REQUIRE_THROWS_WITH(insert_sql_parser::rows_from_line(s, t, v), "Invalid insert query: Rows schema cannot be empty.");
}

TEST_CASE("rows_from_line throws if row closing bracket is missing") {
	std::string s = "INTO Sample {(1,2,3}";
	table t = simple_table();
	std::vector<row> v;


	REQUIRE_THROWS_WITH(insert_sql_parser::rows_from_line(s, t, v), "Invalid insert query: Rows must be described as \"(value, ..., value)\".");
}

TEST_CASE("rows_from_line throws if there is text after \"}\"") {
	std::string s = "INTO Sample {(1,2,3)} as 2, 3aff afe";
	table t = simple_table();
	std::vector<row> v;


	REQUIRE_THROWS_WITH(insert_sql_parser::rows_from_line(s, t, v), "Invalid insert query: There is unparsable text after end of rows.");
}

TEST_CASE("rows_from_line throws if the line is random text") {
	std::string s = "gd, af hs\\ hzr45haf 1752526 gsd INTO ";
	table t = simple_table();
	std::vector<row> v;

	REQUIRE_THROWS_AS(insert_sql_parser::rows_from_line(s, t, v), std::invalid_argument);

	s = "AFAIHF { ;siafh;} jiahfp";
	REQUIRE_THROWS_AS(insert_sql_parser::rows_from_line(s, t, v), std::invalid_argument);

	s = "SomeTableName (field1:String DEFAULT \"Hellooo, how \"are\" you?\", field2:Int, field21:String DEFAULT \"Some text, that contains \"quotes, commas, and more quotes: \" haha\"  , field3:Date DEFAULT 1949-10-27, field4:Double DEFAULT 89.7, field5:DateTime DEFAULT 2035-05-13 07:38:43)";
	REQUIRE_THROWS_AS(insert_sql_parser::rows_from_line(s, t, v), std::invalid_argument);
}