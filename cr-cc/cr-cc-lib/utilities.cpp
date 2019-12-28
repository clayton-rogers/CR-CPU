#include "utilities.h"

#include <sstream>
#include <iomanip>

std::uint16_t to_uint(const std::string& instruction)
{
	const int value = std::stoi(instruction, 0, 16);

	return static_cast<std::uint16_t>(value);
}

std::string u16_to_string(std::uint16_t value) {
	std::stringstream output;
	output << std::hex << std::setfill('0') << std::setw(4) << value;
	std::string temp = output.str();
	for (auto& c : temp) c = static_cast<char>(std::toupper(c));
	return temp;
}

std::string u8_to_string(std::uint8_t value) {
	std::stringstream output;
	output << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(value);// doesn't actually matter what we cast it as, as long as it's not char
	std::string temp = output.str();
	for (auto& c : temp) c = static_cast<char>(std::toupper(c));
	return temp;
}

std::string machine_inst_to_simple_hex(const std::vector<std::uint16_t>& machine_instructions) {

	std::ostringstream ss;

	for (const auto& m : machine_instructions) {
		ss << u16_to_string(m) + " ";
	}

	return ss.str();
}

std::string machine_inst_to_hex(const std::vector<std::uint16_t>& machine_instructions) {

	std::ostringstream ss;

	const int MAX_LINE_LENGTH = 16;

	int i = 0;
	for (const auto& m : machine_instructions) {
		if ((i % MAX_LINE_LENGTH) == 0) {
			// Print leading bit
			ss << "@" << std::hex << std::setfill('0') << std::setw(8) << i << " ";
		}

		ss << u16_to_string(m);
		i++;

		if ((i % MAX_LINE_LENGTH) == 0) {
			ss << std::endl;
		} else {
			ss << " ";
		}
	}
	return ss.str();
}


std::string machine_inst_to_srec(const std::vector<std::uint16_t>& machine_instructions, std::uint16_t address) {
	// Line format of srec is:
	// S0 0F 0000 68656C6C6F20202020200000 3C
	// type, num bytes, addr, data, check sum
	auto write_line = [](int rec_type, std::uint16_t addr, std::vector<std::uint8_t> data) -> std::string {
		std::ostringstream output;
		output << "S";
		output << std::to_string(rec_type);

		std::uint8_t sum = 0;

		std::uint8_t size = static_cast<std::uint8_t>( 2/*addr*/ + data.size() + 1/*checksum*/ );

		output << u8_to_string(size);
		sum += size;

		std::uint8_t high_addr = addr >> 8;
		std::uint8_t low_addr = addr & 0xFF;

		output << u8_to_string(high_addr);
		sum += high_addr;
		output << u8_to_string(low_addr);
		sum += low_addr;

		for (int i = 0; i < data.size(); ++i) {
			output << u8_to_string(data.at(i));
			sum += data.at(i);
		}

		sum ^= 0xFF;
		output << u8_to_string(sum);

		return output.str();
	};

	std::ostringstream ss;

	// Header
	ss << write_line(0, 0, { 0x43,0x52,0x2D,0x43,0x50,0x55,0x00 }) << '\n'; // "CR-CPU"

	// Length
	const int length = static_cast<int>(machine_instructions.size() / 16 + 1); // 32 bytes per line, 16 words

	// Data
	for (int i = 0; i < length; ++i) {
		std::vector<std::uint8_t> data;
		for (int j = 0; j < 16; ++j) {
			int offset = i * 16 + j;
			if (offset == machine_instructions.size()) {
				break;
			}
			data.push_back(machine_instructions.at(offset) >> 8);
			data.push_back(machine_instructions.at(offset) & 0xFF);
		}
		ss << write_line(1, static_cast<uint16_t>(i * 16 + address), data) << '\n';
	}

	// Length
	ss << write_line(5, static_cast<std::uint16_t>(length), {}) << '\n';
	// Finish
	ss << write_line(9, 0, {}) << '\n';

	return ss.str();
}
