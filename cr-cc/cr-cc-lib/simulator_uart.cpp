#include "simulator_uart.h"

void Simulator_Uart::step() {

	auto read_addr = bus->slave_get_read_addr();

	if (read_addr == base_addr + 0x0) {
		// tx size
		bus->slave_return_data(0x0000);
	} else if (read_addr == base_addr + 0x1) {
		// rx size
		bus->slave_return_data(0x0000);
	} else if (read_addr == base_addr + 0x3) {
		// rx data
		bus->slave_return_data(0x0000);
	}

	if (bus->slave_is_write_strobe() &&
		bus->slave_get_write_addr() == base_addr + 0x2) {
		
		// get the data to show that we're handling it, but don't do anything with it
		bus->slave_get_write_data();
	}
}
