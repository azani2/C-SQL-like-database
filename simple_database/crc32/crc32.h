#pragma once
#ifndef CRC32_HEADER
#define CRC32_HEADER
#include <stddef.h>
#include <stdint.h>
#include <array>

class crc32 {
private:
	static constexpr std::array<uint32_t, 256> make_table() noexcept;
	static const std::array<uint32_t, 256> CRC_TABLE;

public:
	static uint32_t generate_crc32_slow(const char*, size_t);
	static uint32_t generate_crc32_fast(const char*, size_t) noexcept;
};

#endif
