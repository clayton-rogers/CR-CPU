#pragma once

#include "utilities.h"
#include "simulator_bus.h"

#include <cstdint>
#include <memory>
#include <array>
#include <vector>

class Simulator_Ram {
public:
	Simulator_Ram(std::shared_ptr<Simulator_Bus> bus)
		: bus(bus)
	{}

	void load_ram(std::uint16_t addr, const std::vector<std::uint16_t>& machine_code);

	void step();

private:
	std::shared_ptr<Simulator_Bus> bus;
	std::array<std::uint16_t, 0x1000> ram = { 0 };
};
