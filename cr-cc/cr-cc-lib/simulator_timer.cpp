#include "simulator_timer.h"


void Simulator_Timer::step() {
	if (timer_low == 0xFFFF) {
		timer_low = 0;
		timer_high += 1;
	} else {
		timer_low += 1;
	}

	if (bus->slave_get_read_addr() == base_addr) {
		bus->slave_return_data(timer_low);
	}

	if (bus->slave_get_read_addr() == base_addr + 1) {
		bus->slave_return_data(timer_high);
	}
}
