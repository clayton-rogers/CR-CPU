#include "simulator_io.h"


void Simulator_IO::step()
{
	if (bus->slave_get_read_addr() == base_addr) {
		bus->slave_return_data(input);
	}

	if (bus->slave_is_write_strobe() &&
		bus->slave_get_write_addr() == base_addr + 1) {
		output = bus->slave_get_write_data();
	}
}
