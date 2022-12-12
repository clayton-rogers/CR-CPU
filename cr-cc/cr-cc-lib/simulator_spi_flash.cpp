#include "simulator_spi_flash.h"

#include <stdexcept>
#include <string>
#include <array>

using BYTE = std::uint8_t;

enum OPCODES {
	READ_ARRAY = 0x03,
	FAST_READ_ARRAY = 0x0B,
	BLOCK_ERASE_4K = 0x20,
	WRITE = 0x02,
	WRITE_ENABLE = 0x06,
	WRITE_DISABLE = 0x04,
	READ_STATUS_BYTE_1 = 0x05,
	READ_MF_ID = 0x9F,
};

void Simulator_Spi_Flash::step() {
	if (busy_count != 0) busy_count--;
}

void Simulator_Spi_Flash::transfer(std::vector<std::uint8_t>* transfer) {
	BYTE opcode = transfer->at(0);

	auto get_addr = [transfer]() -> int {
		int addr = transfer->at(1) << 16 | transfer->at(2) << 8 | transfer->at(3);
		if (addr >= 0x100000 || addr < 0x50000) {
			throw std::logic_error("Tried to access a SPI flash address out of range: " + std::to_string(addr));
		}
		return addr;
	};

	if (busy_count != 0 && opcode != READ_STATUS_BYTE_1) {
		throw std::logic_error("Tried to access spi flash while it was busy. OP: " + std::to_string(opcode));
	}

	switch (opcode) {
	case READ_ARRAY:
	{
		const int address = get_addr();
		const int size = (int)transfer->size() - 4;

		for (int i = 0; i < size; ++i) {
			transfer->at(i + 4) = data.at(address + i);
		}
		break;
	}

	case FAST_READ_ARRAY:
		throw std::logic_error("FAST READ ARRAY not implemented");
		break;

	case BLOCK_ERASE_4K:
	{
		if (!WEL_bit) {
			return;
		}
		const int address = get_addr() & 0xFFFFF000;
		for (int i = 0; i < 4 * 1024; ++i) {
			data.at(address + i) = 0xFF;
			data_erased.at(address + i) = true;
		}

		WEL_bit = false;

		busy_count = 512; // make flash busy for a while after erasing

		break;
	}

	case WRITE:
	{
		if (transfer->size() > 256 + 3 + 1) {
			throw std::logic_error("Write transfer size too large");
		}
		if (!WEL_bit) {
			return;
		}

		const int address = get_addr();
		for (int i = 0; i < static_cast<int>(transfer->size()) - 4; ++i) {
			if (!data_erased.at(address + i)) {
				throw std::logic_error("Tried to write to an already written location: " + std::to_string(address + i));
			}

			data.at(address + i) = transfer->at(i + 4);
			data_erased.at(address + i) = false;
		}

		WEL_bit = false;

		busy_count = 512; // make flash busy for a while after writing

		break;
	}

	case WRITE_ENABLE:
		WEL_bit = true;
		break;

	case WRITE_DISABLE:
		WEL_bit = false;
		break;

	case READ_STATUS_BYTE_1:
	{
		BYTE ret_val = (WEL_bit ? 0x2 : 0x0) | (busy_count != 0 ? 0x01 : 0x0);
		// Technically reading any number is legal, but we are not going to allow it
		if (transfer->size() != 2) { throw std::logic_error("Tried to read more than one byte of status 1"); }
		transfer->at(1) = ret_val;
		break;
	}

	case READ_MF_ID:
	{
		const std::array<BYTE, 3> mf_id = { 0x1F, 0x85, 0x01 };
		for (int i = 0; i < static_cast<int>(mf_id.size()); ++i) {
			transfer->at(i + 1) = mf_id.at(i);
		}
		break;
	}

	default:
		throw std::logic_error("Unknown opcode: " + std::to_string((int)opcode));
	}
}
