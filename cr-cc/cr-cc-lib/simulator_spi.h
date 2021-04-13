#pragma once

#include "simulator_bus.h"
#include "simulator_spi_flash.h"

#include <cstdint>
#include <memory>
#include <vector>

// MEMORY MAP
// 0x00 = data (read and write)
// 0x01 = data addr
// 0x02 = status (0x01 = start tx/busy)
// 0x03 = transfer_size

class Simulator_Spi {
public:
	Simulator_Spi(std::shared_ptr<Simulator_Bus> bus, std::uint16_t base_addr)
		: bus(bus), base_addr(base_addr), internal_buffer(512, 0x00)
	{ }

	void step();

private:
	std::shared_ptr<Simulator_Bus> bus;
	std::uint16_t base_addr;

	std::vector<std::uint8_t> internal_buffer;
	int buffer_ptr = 0;
	int transfer_count_down = 0;
	int transfer_size = 0;

	Simulator_Spi_Flash flash;
};