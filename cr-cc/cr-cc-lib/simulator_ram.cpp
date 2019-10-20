#include "simulator_ram.h"

void Simulator_Ram::step() {
	if (bus->read_addr < ram_size) {
		bus->read_data = ram.at(bus->read_addr);
	}

	if (bus->write_strobe && bus->write_addr < ram_size) {
		ram.at(bus->write_addr) = bus->write_data;
	}
}