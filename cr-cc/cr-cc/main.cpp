#include "assembler.h"
#include "file_io.h"
#include "machine_io.h"

#include <iostream>
#include <string>
#include <vector>

const std::string DEFAULT_OUTPUT_FILENAME("out.hex");

int main(int argc, char **argv) {
	
	if (argc != 2) {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	std::string filename(argv[1]);

	std::cout << "Filename: " << filename << std::endl;

	std::string file_contents = read_file(filename);

	std::cout << "\nFile contents: \n" << file_contents << std::endl;

	try {
		const auto machine_code = assemble(file_contents);
		std::string formatted = machine_inst_to_formated(machine_code);
		std::cout << "\nMachine code: \n" << formatted << std::endl;
		write_file(DEFAULT_OUTPUT_FILENAME, formatted);
	} catch (std::logic_error& e) {
		std::cout << "ASSEMBLY ERROR: \n";
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
