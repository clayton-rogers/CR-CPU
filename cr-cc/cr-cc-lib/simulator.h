#pragma once

#include "simulator_bus.h"
#include "simulator_core.h"
#include "simulator_io.h"
#include "simulator_ram.h"
#include "simulator_timer.h"

#include <cstdint>
#include <vector>

class Simulator {
public:
	Simulator()
		: bus(std::make_shared<Simulator_Bus>()),
		ram(bus),
		io(bus, 0x8100),
		timer(bus, 0x8200),
		core(bus) {}

	struct State {
		std::uint16_t ra;
		std::uint16_t pc;

		bool is_halted;

		std::uint16_t output;
	};
	void load(std::uint16_t addr, std::vector<std::uint16_t> machine_code);

	void step();
	void run_until_halted(int number_steps);

	State get_state();

private:
	std::shared_ptr<Simulator_Bus> bus;
	Simulator_Ram ram;
	Simulator_IO io;
	Simulator_Timer timer;
	Simulator_Core core;
};
