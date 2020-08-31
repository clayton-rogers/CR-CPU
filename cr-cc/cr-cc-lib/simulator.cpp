#include "simulator.h"

void Simulator::load(const Object::Object_Container& obj) {
	if (obj.contents.index() != Object::Object_Container::EXECUTABLE) {
		throw std::logic_error("Tried to load invalid object into simulator");
	}
	const auto exe = std::get<Object::Executable>(obj.contents);
	ram.load_ram(exe.load_address, exe.machine_code);
}

void Simulator::step() {
	core.step();
	ram.step();
	io.step();
	timer.step();
}

void Simulator::run_until_halted(const int number_steps) {
	steps_remaining = number_steps;
	while (!get_state().is_halted && steps_remaining != 0) {
		--steps_remaining;
		step();
	}
}

Simulator::State Simulator::get_state() {
	State s{
		core.get_ra(),
		core.pc,
		core.is_halted,

		io.output,
		steps_remaining,
	};

	return s;
}
