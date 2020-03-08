#include "simulator.h"

void Simulator::load(std::uint16_t addr, std::vector<std::uint16_t> machine_code) {
	ram.load_ram(addr, machine_code);
}

void Simulator::step() {
	core.step();
	ram.step();
	io.step();
	timer.step();
}

void Simulator::run_until_halted(int number_steps) {
	while (!get_state().is_halted && number_steps != 0) {
		--number_steps;
		step();
	}
}

Simulator::State Simulator::get_state() {
	State s{
		core.get_ra(),
		core.pc,
		core.is_halted,

		io.output,
	};

	return s;
}
