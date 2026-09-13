#include "simulator_vga.h"

void Simulator_Vga::step()
{
	if (bus->slave_is_write_strobe()) {
		if (bus->slave_get_write_addr() == base_addr ||
			bus->slave_get_write_addr() == base_addr + 1) {
			// Read the data but don't do anything with it
			// Tells the bus that we have handled the request
			bus->slave_get_write_data();
		}
	}
}
