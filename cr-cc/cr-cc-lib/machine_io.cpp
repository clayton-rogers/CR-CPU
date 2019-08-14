#include "machine_io.h"

#include <sstream>
#include <iomanip>

std::string machine_inst_to_unformatted(const std::vector<std::string>& machine_instructions) {
	
	std::ostringstream ss;

	for (const auto& m : machine_instructions) {
		ss << m + " ";
	}

	return ss.str();
}

std::string machine_inst_to_formated(const std::vector<std::string>& machine_instructions) {

	std::ostringstream ss;

	const int MAX_LINE_LENGTH = 16;

	int i = 0;
	for (const auto& m : machine_instructions) {
		if ((i % MAX_LINE_LENGTH) == 0) {
			// Print leading bit
			ss << "@" << std::hex << std::setfill('0') << std::setw(8) << i << " ";
		}

		ss << m;
		i++;

		if ((i%MAX_LINE_LENGTH) == 0) {
			ss << std::endl;
		} else {
			ss << " ";
		}
	}
	return ss.str();
}