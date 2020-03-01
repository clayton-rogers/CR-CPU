#include "compiler.h"
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
	
	if (argc != 2) {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	try {
		std::string filename(argv[1]);
		std::string file_extension = filename.substr(filename.find_last_of(".") + 1);

		std::string assembly_code;
		if (file_extension == "c") {
			FileReader f;
			// TODO if any include paths are specified add them here.
			assembly_code = compile(filename, f);
			write_file(DEFAULT_OUTPUT_FILENAME + "s", assembly_code);
		} else if (file_extension == "s") {
			assembly_code = read_file(filename);
		} else {
			throw std::logic_error("Cannot handle file with extension: " + file_extension);
		}

		std::uint16_t offset = 0;
		const auto machine_code = assemble(assembly_code, &offset);
		//std::cout << "Code size: " << machine_code.size() << "/" << RAM_SIZE_WORDS
		//	<< " (" << std::fixed << std::setprecision(2)
		//	<< static_cast<float>(machine_code.size()) / RAM_SIZE_WORDS * 100 << "%)"
		//	<< std::endl;

		write_file(DEFAULT_OUTPUT_FILENAME + "hex", machine_inst_to_hex(machine_code));
		write_bin_file(DEFAULT_OUTPUT_FILENAME + "bin", machine_code);
		write_file(DEFAULT_OUTPUT_FILENAME + "srec", machine_inst_to_srec(machine_code, offset));

		// TODO temp output srec to terminal
		//std::cout << "\n"
		//	<< machine_inst_to_srec(machine_code, offset);

	} catch (std::logic_error& e) {
		std::cout << "ASSEMBLY ERROR: \n";
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
