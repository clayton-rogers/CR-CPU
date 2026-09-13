#pragma once

#include "simulator_bus.h"

#include <cstdint>
#include <memory>

// 0x00 = tx size
// 0x01 = rx size
// 0x02 = tx data
// 0x03 = rx data

class Simulator_Uart {
public:
	Simulator_Uart(std::shared_ptr<Simulator_Bus> bus, std::uint16_t base_addr)
		: bus(bus), base_addr(base_addr)
	{}

	void step();

private:
	std::shared_ptr<Simulator_Bus> bus;
	std::uint16_t base_addr;
};
