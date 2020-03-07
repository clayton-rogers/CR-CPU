#include "compiler.h"
#include "file_io.h"
#include "utilities.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

const std::string DEFAULT_OUTPUT_FILENAME("out.");
const int RAM_SIZE_WORDS = 4096;

struct Options {
	bool verbose = false;
};

static Options opt;

int main(int argc, char **argv) {
	
	if (argc != 2) {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	try {
		std::string filename(argv[1]);
		FileReader f;

		auto ret = compile_tu(filename, f);
		write_file(DEFAULT_OUTPUT_FILENAME + "s", ret.assembly);


		write_file(DEFAULT_OUTPUT_FILENAME + "hex", machine_inst_to_hex(ret.machine_code));
		write_bin_file(DEFAULT_OUTPUT_FILENAME + "bin", ret.machine_code);
		auto srec = machine_inst_to_srec(ret.machine_code, ret.load_address);
		write_file(DEFAULT_OUTPUT_FILENAME + "srec", srec);

		if (opt.verbose) {
			std::cout << "Code size: " << ret.machine_code.size() << "/" << RAM_SIZE_WORDS
				<< " (" << std::fixed << std::setprecision(2)
				<< static_cast<float>(ret.machine_code.size()) / RAM_SIZE_WORDS * 100 << "%)"
				<< std::endl;

			std::cout << "\n" << srec;
		}

	} catch (std::logic_error& e) {
		std::cout << "ASSEMBLY ERROR: \n";
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
