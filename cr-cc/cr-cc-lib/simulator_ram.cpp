#include "simulator_ram.h"

void Simulator_Ram::step() {

	if (bus->slave_get_read_addr() < ram.size()) {
		bus->slave_return_data(ram.at(bus->slave_get_read_addr()));
	}

	if (bus->slave_is_write_strobe() &&
		bus->slave_get_write_addr() < ram.size()) {

		ram.at(bus->slave_get_write_addr()) = bus->slave_get_write_data();
	}
}

void Simulator_Ram::load_ram(std::uint16_t addr, const std::vector<std::uint16_t>& machine_code) {
	for (std::uint16_t i = 0; i < machine_code.size(); ++i) {
		ram.at(i + addr) = machine_code.at(i);
	}
}
