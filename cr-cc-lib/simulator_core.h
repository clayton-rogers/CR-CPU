#pragma once

#include "simulator_bus.h"

#include <cstdint>
#include <vector>
#include <string>
#include <memory>


class Simulator_Core {
public:

	Simulator_Core(std::shared_ptr<Simulator_Bus> bus)
		: bus(bus)
	{}
	Simulator_Core() = delete;

	void step();

	// **** PUBLIC DATA *** //
	bool is_halted = false;

	struct Reg {
		std::uint16_t ra = 0, rb = 0, rp = 0, sp = 0;
	};
	Reg get_reg() { return Reg{ ra, rb, rp, sp }; }
	std::uint16_t get_pc() { return pc; }

private:
	// **** DATA **** //
	std::uint16_t pc = 0xFFFF;
	std::uint16_t ra = 0, rb = 0, rp = 0, sp = 0;
	std::uint8_t addr = 0;
	std::uint16_t cached_ins = 0;

	int state = 0;

	std::shared_ptr<Simulator_Bus> bus;

	// **** FUNCTIONS **** //
	std::uint16_t& get_reg(int index);
};
