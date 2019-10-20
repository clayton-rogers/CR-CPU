#pragma once

#include <cstdint>

class Simulator_Bus {
public:
	std::uint16_t read_addr;
	std::uint16_t read_data;

	std::uint16_t write_addr;
	std::uint16_t write_data;
	bool write_strobe;
};