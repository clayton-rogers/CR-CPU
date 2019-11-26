#include "assembler.h"
#include "file_io.h"
#include "utilities.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

const std::string DEFAULT_OUTPUT_FILENAME("out.");
const int RAM_SIZE_WORDS = 4096;

int main(int argc, char **argv) {
	
	if (argc < 2) {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	std::string filename(argv[1]);
	std::string file_contents = read_file(filename);

	std::uint16_t address = 0;
	if (argc >= 3) {
		address = static_cast<std::uint16_t>(std::stoi(argv[2], 0, 0));
	}

	try {
		const auto machine_code = assemble(file_contents);
		std::cout << "Code size: " << machine_code.size() << "/" << RAM_SIZE_WORDS
			<< " (" << std::fixed << std::setprecision(2)
			<< static_cast<float>(machine_code.size()) / RAM_SIZE_WORDS * 100 << "%)"
			<< std::endl;

		std::string hex = machine_inst_to_hex(machine_code);
		write_file(DEFAULT_OUTPUT_FILENAME + "hex", hex);

		write_bin_file(DEFAULT_OUTPUT_FILENAME + "bin", machine_code);

		write_file(DEFAULT_OUTPUT_FILENAME + "srec", machine_inst_to_srec(machine_code, address));

	} catch (std::logic_error& e) {
		std::cout << "ASSEMBLY ERROR: \n";
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
