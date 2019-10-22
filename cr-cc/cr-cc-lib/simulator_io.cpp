#include "simulator_io.h"


void Simulator_IO::step() {
	if (bus->read_addr == base_addr) {
		bus->read_data = input;
	}

	if (bus->write_strobe && bus->write_addr == base_addr + 1) {
		output = bus->write_data;
	}
}