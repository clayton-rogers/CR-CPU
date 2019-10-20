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