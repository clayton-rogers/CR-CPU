#pragma once

#include "simulator_bus.h"

#include <memory>

// 0x00 = vga data
// 0x01 = cursor location
class Simulator_Vga {
public:

	Simulator_Vga(std::shared_ptr<Simulator_Bus> bus, std::uint16_t base_addr)
		: bus(bus), base_addr(base_addr)
	{}

	void step();

private:
	std::shared_ptr<Simulator_Bus> bus;
	std::uint16_t base_addr;
};
