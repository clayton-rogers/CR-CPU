#pragma once

#include <cstdint>

class Simulator_Bus {
public:
	bool slave_is_write_strobe();
	std::uint16_t slave_get_write_addr();
	std::uint16_t slave_get_write_data();
	std::uint16_t slave_get_read_addr();
	void slave_return_data(std::uint16_t data);

	void core_write_data(std::uint16_t addr, std::uint16_t data);
	void core_set_next_read_address(std::uint16_t addr);
	std::uint16_t core_read_data();

	void check_bus_state_and_reset();

private:
	std::uint16_t read_addr;
	std::uint16_t read_data;
	int read_data_fulfilled = 0;

	std::uint16_t write_addr;
	std::uint16_t write_data;
	bool write_strobe;
	int write_data_fulfilled = 0;
};
