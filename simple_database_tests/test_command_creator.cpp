#include "catch_amalgamated.hpp"
#include "command_creator.h"
#include <stdexcept>

TEST_CASE("type is correctly parsed from a line") {
	REQUIRE(command_creator::com_from_line("CreateTable some     table").type == CREATE);
	REQUIRE(command_creator::com_from_line("DropTable 5678 something 1234").type == DROP);
	REQUIRE(command_creator::com_from_line("Select lalalala").type == SELECT);
	REQUIRE(command_creator::com_from_line("Remove hello ").type == REMOVE);
	REQUIRE(command_creator::com_from_line("Insert 12345678").type == INSERT);
}

TEST_CASE("com_from_line throws when command keyword matches no valid command keyword") {
	REQUIRE_THROWS_AS(command_creator::com_from_line("Selact some data"), std::invalid_argument);
}

TEST_CASE("command data contains no leading and trailing spaces") {
	REQUIRE(command_creator::com_from_line("Select lalalala").data == "lalalala");
	REQUIRE(command_creator::com_from_line("Remove    hello ").data == "hello");
	REQUIRE(command_creator::com_from_line("DropTable 5678 something 1234").data == "5678 something 1234");
}