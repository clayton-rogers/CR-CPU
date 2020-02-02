#pragma once

#include "simulator_bus.h"

#include <cstdint>
#include <memory>


// Simulated 32 bit timer - increments every clock
//   BASE_ADDR     => least significant word
//   BASE_ADDR + 1 => most significant word
class Simulator_Timer {
public:
	Simulator_Timer(std::shared_ptr<Simulator_Bus> bus, std::uint16_t base_addr) :
		bus(bus), base_addr(base_addr) {}

	void step();

private:
	std::shared_ptr<Simulator_Bus> bus;
	std::uint16_t base_addr;

	std::uint16_t timer_low = 0;
	std::uint16_t timer_high = 0;
};
