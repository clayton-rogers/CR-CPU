#pragma once

#include "simulator_bus.h"

#include <cstdint>
#include <memory>


// Simulated discrete IO
//   BASE_ADRR     => input (read only)
//   BASE_ADDR + 1 => output (write only)
class Simulator_IO {
public:
	std::uint16_t output = 0;
	std::uint16_t input = 0;

	Simulator_IO(std::shared_ptr<Simulator_Bus> bus, std::uint16_t base_addr) :
		bus(bus), base_addr(base_addr) {

	}

	void step();

private:
	std::uint16_t base_addr;
	std::shared_ptr<Simulator_Bus> bus;
};