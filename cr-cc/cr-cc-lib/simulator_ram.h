#pragma once

#include "utilities.h"
#include "simulator_bus.h"

#include <cstdint>
#include <memory>
#include <vector>

class Simulator_Ram {
public:
	Simulator_Ram(
		const std::vector<std::string>& input_instructions,
		std::uint16_t size_of_ram,
		std::shared_ptr<Simulator_Bus> bus) :

		ram_size(size_of_ram),
		bus(bus)
	{
		for (const auto& i_str : input_instructions) {
			ram.push_back(to_uint(i_str));
		}
		ram.resize(size_of_ram, 0);
	}

	void step();

private:

	std::uint16_t ram_size;
	std::shared_ptr<Simulator_Bus> bus;
	std::vector<std::uint16_t> ram;
};