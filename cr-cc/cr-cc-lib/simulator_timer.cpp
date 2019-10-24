#include "simulator_timer.h"


void Simulator_Timer::step() {
	if (timer_low == 0xFFFF) {
		timer_low = 0;
		timer_high += 1;
	} else {
		timer_low += 1;
	}

	if (bus->read_addr == base_addr) {
		bus->read_data = timer_low;
	}

	if (bus->read_addr == base_addr + 1) {
		bus->read_data = timer_high;
	}
}
