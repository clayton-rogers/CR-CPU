#pragma once

#include "simulator_bus.h"
#include "simulator_core.h"
#include "simulator_io.h"
#include "simulator_ram.h"
#include "simulator_timer.h"
#include "simulator_uart.h"
#include "simulator_vga.h"

#include "object_code.h"

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

class Simulator {
public:
	Simulator();

	struct State {
		std::uint16_t ra;
		std::uint16_t pc;

		bool is_halted;

		std::uint16_t output;
		int steps_remaining;
	};
	void load(const Object::Object_Container& obj);
	void load_sim_overlay();

	void step(bool output_state = false);
	void run_until_halted(const int number_steps, bool output_state = false);

	State get_state();

private:
	std::shared_ptr<Simulator_Bus> bus;
	Simulator_Core core;
	Simulator_Ram ram;
	Simulator_IO io;
	Simulator_Timer timer;
	Simulator_Uart uart;
	Simulator_Vga vga;
	int steps_remaining = 0;

	std::ofstream f;
	void dump_state();
};
