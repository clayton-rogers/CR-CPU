#include "simulator_spi.h"

#include <stdexcept>
#include <string>

// MEMORY MAP
// 0x00 = data (read and write)
// 0x01 = data addr
// 0x02 = status (0x01 = start tx/busy)
// 0x03 = transfer_size

void Simulator_Spi::step() {

	flash.step();

	auto read_addr = bus->slave_get_read_addr() - base_addr;
	auto write_addr = bus->slave_get_write_addr() - base_addr;

	if (transfer_count_down != 0) {
		--transfer_count_down;

		// When enough time has passed, do the transfer
		if (transfer_count_down == 0) {
			std::vector<std::uint8_t> transfer(transfer_size, 0);

			for (int i = 0; i < transfer_size; ++i) {
				transfer.at(i) = internal_buffer.at(i);
			}

			flash.transfer(&transfer);

			for (int i = 0; i < transfer_size; ++i) {
				internal_buffer.at(i) = transfer.at(i);
			}
		}

		if (read_addr == 0x00 ||
			read_addr == 0x01 ||
			// reading offset 0x02 is the only valid operation while a transfer is in progress
			read_addr == 0x03 ||

			bus->slave_is_write_strobe() && (
				write_addr == 0x00 ||
				write_addr == 0x01 ||
				write_addr == 0x02 ||
				write_addr == 0x03
			)
			) {
			throw std::logic_error("Tried to perform an SPI operation while transfer was in progress. Addr: "
				+ std::to_string(read_addr) + " " + std::to_string(write_addr));
		}
	}

	if (read_addr == 0x00) {
		bus->slave_return_data(internal_buffer.at(buffer_ptr));
		++buffer_ptr;
	} else if (read_addr == 0x01) {
		bus->slave_return_data(static_cast<std::uint16_t>(buffer_ptr));
	} else if (read_addr == 0x02) {
		if (transfer_count_down == 0) {
			bus->slave_return_data(0);
		} else {
			bus->slave_return_data(1);
		}
	} else if (read_addr == 0x03) {
		bus->slave_return_data(static_cast<std::uint16_t>(transfer_size));
	}

	if (bus->slave_is_write_strobe()) {
		if (write_addr == 0x00) {
			auto data = bus->slave_get_write_data();
			internal_buffer.at(buffer_ptr) = static_cast<std::uint8_t>(data);
			++buffer_ptr;
		} else if (write_addr == 0x01) {
			buffer_ptr = bus->slave_get_write_data();
		} else if (write_addr == 0x02) {
			auto data = bus->slave_get_write_data();
			if (data == 0x01) {
				transfer_count_down = transfer_size * 8;
			} else {
				throw std::logic_error("Tried to write to SPI::status with invalid value");
			}
		} else if (write_addr == 0x03) {
			transfer_size = bus->slave_get_write_data();
		}
	}
}
