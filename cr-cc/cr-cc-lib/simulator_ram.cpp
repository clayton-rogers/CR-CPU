#include "simulator_ram.h"

void Simulator_Ram::step() {
	if (bus->read_addr < ram.size()) {
		bus->read_data = ram.at(bus->read_addr);
	}

	if (bus->write_strobe && bus->write_addr < ram.size()) {
		ram.at(bus->write_addr) = bus->write_data;
	}
}

void Simulator_Ram::load_ram(std::uint16_t addr, std::vector<std::uint16_t> machine_code) {
	for (std::uint16_t i = 0; i < machine_code.size(); ++i) {
		ram.at(i + addr) = machine_code.at(i);
	}
}
