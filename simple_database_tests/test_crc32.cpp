#include "catch_amalgamated.hpp"
#include "crc32.h"

std::vector<std::pair<std::string, uint32_t>> CRC_TEST_VALUES = {
	{ "123456789", 0xcbf43926 },
	{ "The quick brown fox jumps over the lazy dog", 0x414fa339 }
};

TEST_CASE("generate_crc32_slow correctly generate crc for standart strings test") {
	for (size_t i = 0; i < CRC_TEST_VALUES.size(); i++) {
		REQUIRE(crc32::generate_crc32_slow(CRC_TEST_VALUES[i].first.data(), CRC_TEST_VALUES[i].first.length()) == CRC_TEST_VALUES[i].second);
	}
}

TEST_CASE("generate_crc32_fast correctly generate crc for standart strings test") {
	for (size_t i = 0; i < CRC_TEST_VALUES.size(); i++) {
		REQUIRE(crc32::generate_crc32_fast(CRC_TEST_VALUES[i].first.data(), CRC_TEST_VALUES[i].first.length()) == CRC_TEST_VALUES[i].second);
	}
}