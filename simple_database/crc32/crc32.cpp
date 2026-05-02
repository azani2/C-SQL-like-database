#include "crc32.h"
//TODO: add credits

//CRC-32/ISO-HDLC

//slow, no lookup table
uint32_t crc32::generate_crc32_slow(const char* data, size_t size) {
	uint32_t crc = 0xFFFFFFFF;

	for (size_t i = 0; i < size; i++) {
		char c = data[i];
		for (size_t j = 0; j < 8; j++) {
			uint32_t b = (c ^ crc) & 1;
			crc >>= 1;
			if (b) {
				crc = crc ^ 0xEDB88320; //generator polynomial
			}
			c >>= 1;
		}
	}
	return ~crc;
}

//with lookup table
constexpr std::array<uint32_t, 256> crc32::make_table() noexcept {
	std::array<uint32_t, 256> table{};
	for (size_t i = 0; i < 256; i++) {
		uint32_t curr = i;
		for (uint8_t j = 0; j < 8; j++) {
			if (curr & 1) {
				curr = uint32_t{ 0xEDB88320 } ^ (curr >> 1);
			}
			else {
				curr >>= 1;
			}
		}
		table[i] = curr;
	}
	return table;
}

const std::array<uint32_t, 256> crc32::CRC_TABLE = crc32::make_table();

uint32_t crc32::generate_crc32_fast(const char* data, size_t size) noexcept {
	uint32_t crc = 0xFFFFFFFFu;
	for (size_t i = 0; i < size; i++) {
		uint8_t byte = static_cast < uint8_t > (data[i]);
		crc = CRC_TABLE[(crc ^ byte) & 0xFF] ^ (crc >> 8);
	}
	return ~crc;
}