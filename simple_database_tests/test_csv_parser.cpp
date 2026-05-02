#include "catch_amalgamated.hpp"
#include "csv_parser.h"
#include "test_utils.h"

TEST_CASE("tb_info_from_csv correctly parses table_info") {
	std::string s = "TestTable,4,3,1,0,12";
	table_info deserialized = csv_parser::tb_info_from_csv(s);
	compare_tbi_to(deserialized, "TestTable", 4, 3, true, 0, 12);
}

TEST_CASE("tb_info_to_csv correctly converts table_info to string") {
	table_info tbi;
	tbi.table_name = "TestTable";
	tbi.col_count = 12;
	tbi.row_count = 150893;
	tbi.is_indexed = false;
	tbi.index_column_index = 4;
	tbi.max_index = 1087452;
	std::string expected = "TestTable,12,150893,0,4,1087452";
	REQUIRE(csv_parser::tb_info_to_csv(tbi) == expected);
}

TEST_CASE("deserialization with tb_info_from_csv after serialization with tb_info_to_csv returns the same table_info object") {
	table_info tbi;
	tbi.table_name = "TestTable";
	tbi.col_count = 12;
	tbi.row_count = 150893;
	tbi.is_indexed = false;
	tbi.index_column_index = 4;
	tbi.max_index = 0;

	std::string ser = csv_parser::tb_info_to_csv(tbi);
	table_info tbi2 = csv_parser::tb_info_from_csv(ser);
	compare_tbi_to(tbi2, tbi.table_name, 12, 150893, false, 4, 0);
}

TEST_CASE("row_from_csv correctly parses row") {
	std::vector<col_info> cols = {
		col_info{"Name",col_type::STRING,true,"Unknown (?)",false,0},
		col_info{"Score",col_type::DOUBLE,true,0.0,false,1},
		col_info{"ID",col_type::INT,false,0,true,2}
	};
	std::string row_s = "Didn't share\\, but is called Garry,15.5,1";
	row expected;
	expected.push_back("Didn't share, but is called Garry");
	expected.push_back(15.5);
	expected.push_back(1);
	compare_row(expected, csv_parser::row_from_csv(row_s, cols));
}

TEST_CASE("row_from_csv correctly parses row with escaped comma in the middle") {
	std::vector<col_info> cols = {
		col_info{"ID",col_type::INT,false,0,true,0},
		col_info{"Name",col_type::STRING,true,"Unknown (?)",false,1},
		col_info{"Score",col_type::DOUBLE,true,0.0,false,2}
		
	};
	std::string row_s = "1,Didn't share\\, but is called Garry,15.5";
	row expected;
	expected.push_back(1);
	expected.push_back("Didn't share, but is called Garry");
	expected.push_back(15.5);
	compare_row(expected, csv_parser::row_from_csv(row_s, cols));
}

TEST_CASE("row_from_csv parses escaped comma at beginning of field") {
	std::vector<col_info> cols = {
		col_info{"A",col_type::STRING,false,"",false,0 }
	};
	std::string row_s = "\\,starts with comma";
	row expected;
	expected.push_back(",starts with comma");

	compare_row(expected, csv_parser::row_from_csv(row_s, cols));
}

TEST_CASE("row_from_csv parses escaped comma at the end of the line") {
	std::vector<col_info> cols = {
		col_info{"Col1",col_type::STRING,false,"",false,0 },
		col_info{"Col2",col_type::STRING,false,"",false,1 }
	};

	std::string row_s = "something\\, \"something\",end\\,";

	row expected;
	expected.push_back("something, \"something\"");
	expected.push_back("end,");

	compare_row(expected, csv_parser::row_from_csv(row_s, cols));
}

TEST_CASE("row_from_csv parses single column row") {
	std::vector<col_info> cols = {
		col_info{"A",col_type::INT,false,0,false,0 }
	};
	std::string row_s = "10";
	row expected;
	expected.push_back(10);

	compare_row(expected, csv_parser::row_from_csv(row_s, cols));
}

TEST_CASE("row_from_csv parses simple row") {
	std::vector<col_info> cols = {
		col_info{"A",col_type::INT,false,0,false,0 },
		col_info{"B",col_type::DOUBLE,false,0.0,false,1 },
		col_info{"C",col_type::STRING,false,"",false,2 }
	};
	std::string row_s = "10,3.14,hello";
	row expected;
	expected.push_back(10);
	expected.push_back(3.14);
	expected.push_back("hello");

	compare_row(expected, csv_parser::row_from_csv(row_s, cols));
}

TEST_CASE("row_from_csv parses empty string columns") {
	std::vector<col_info> cols = {
		col_info{"Col1",col_type::STRING,false,"",false,0 },
		col_info{"Col2",col_type::STRING,false,"",false,1 },
		col_info{"Col3",col_type::STRING,false,"",false,2 },
		col_info{"Col3",col_type::STRING,false,"",false,3 },
		col_info{"Col5",col_type::STRING,false,"",false,4 }
	};
	std::string row_s = ",not empty,,not empty,";
	row expected;
	expected.push_back("");
	expected.push_back("not empty");
	expected.push_back("");
	expected.push_back("not empty");
	expected.push_back("");

	compare_row(expected, csv_parser::row_from_csv(row_s, cols));
}

TEST_CASE("row_from_csv parses negative numbers") {
	std::vector<col_info> cols = {
		col_info{"IntColumn",col_type::INT,false,0,false,0 },
		col_info{"DoubleColumn",col_type::DOUBLE,false,0.0,false,1 }
	};
	std::string row_s = "-2,-3.5";
	row expected;
	expected.push_back(-2);
	expected.push_back(-3.5);

	compare_row(expected, csv_parser::row_from_csv(row_s, cols));
}

TEST_CASE("row_from_csv throws when there are less fields than columns in vector") {
	std::vector<col_info> cols = {
		col_info{"A",col_type::INT,false,0,false,0 },
		col_info{"B",col_type::DOUBLE,false,0.0,false,1 },
		col_info{"C",col_type::STRING,false,"",false,2 }
	};
	std::string row_s = "-2,-3.5";

	REQUIRE_THROWS_WITH(csv_parser::row_from_csv(row_s, cols), "There are not enough fields in this csv record:\n"
		+ row_s + "\nFound: 2, expected: 3 fields.");
}

TEST_CASE("row_from_csv throws when there are more fields than columns in vector") {
	std::vector<col_info> cols = {
		col_info{"A",col_type::INT,false,0,false,0 },
		col_info{"B",col_type::DOUBLE,false,0.0,false,1 },
		col_info{"C",col_type::STRING,false,"",false,2 }
	};
	std::string row_s = "-2,-3.5,Something something,Heyy";

	REQUIRE_THROWS_WITH(csv_parser::row_from_csv(row_s, cols), "There are too many fields in this csv record:\n"
		+ row_s + "\nFound: 4, expected: 3 fields.");
}