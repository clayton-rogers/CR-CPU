#pragma once

#include "simulator_bus.h"

#include <cstdint>
#include <vector>
#include <string>
#include <memory>


class Simulator {
public:

	Simulator(std::shared_ptr<Simulator_Bus> bus) : bus(bus) {}
	Simulator() = delete;

	void step();

	// **** PUBLIC DATA *** //
	std::uint16_t output = 0;
	std::uint16_t input = 0;
	std::uint16_t pc = 0xFFFF;
	bool is_halted = false;

private:
	// **** DATA **** //
	std::uint16_t ra = 0, rb = 0, bp = 0, sp = 0;
	std::uint8_t addr = 0;
	std::uint16_t cached_ins = 0;

	int state = 0;

	std::shared_ptr<Simulator_Bus> bus;

	// **** FUNCTIONS **** //
	std::uint16_t& get_reg(int index);
};