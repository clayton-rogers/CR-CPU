#include "simulator_bus.h"

#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>

void Simulator_Bus::check_bus_state_and_reset() {
	std::stringstream r_ss;
	r_ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << read_addr;
	auto r_addr = r_ss.str();

	std::stringstream w_ss;
	w_ss << "0x" << std::hex << std::setfill('0') << std::setw(4) << write_addr;
	auto w_addr = w_ss.str();

	if (0 == read_data_fulfilled) {
		throw std::logic_error("Tried to read an invalid address: " + r_addr);
	}
	if (read_data_fulfilled > 1) {
		throw std::logic_error("Multiple slaves responded to read address: " + r_addr);
	}
	if (write_strobe && 0 == write_data_fulfilled) {
		throw std::logic_error("Tried to write an invalid address: " + w_addr);
	}
	if (write_strobe && write_data_fulfilled > 1) {
		throw std::logic_error("Multiple slaves responded to write address: " + w_addr);
	}
	if (!write_strobe && write_data_fulfilled != 0) {
		throw std::logic_error("Slave responded to write without strobe: " + w_addr);
	}

	read_data_fulfilled = 0;
	write_data_fulfilled = 0;
	write_strobe = false;
}

bool Simulator_Bus::slave_is_write_strobe() {
	return write_strobe;
}

std::uint16_t Simulator_Bus::slave_get_write_addr() {
	return write_addr;
}

std::uint16_t Simulator_Bus::slave_get_write_data() {
	// We assume that if the slave is getting the write data that they're going to use it
	write_data_fulfilled++;
	return write_data;
}

std::uint16_t Simulator_Bus::slave_get_read_addr() {
	return read_addr;
}

void Simulator_Bus::slave_return_data(std::uint16_t data) {
	read_data = data;
	read_data_fulfilled++;
}

void Simulator_Bus::core_write_data(std::uint16_t addr, std::uint16_t data) {
	write_strobe = true;
	write_addr = addr;
	write_data = data;
}

void Simulator_Bus::core_set_next_read_address(std::uint16_t addr) {
	read_addr = addr;
}

std::uint16_t Simulator_Bus::core_read_data() {
	return read_data;
}

