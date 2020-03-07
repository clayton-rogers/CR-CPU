#include "compiler.h"
#include "file_io.h"
#include "utilities.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

static const std::string DEFAULT_OUTPUT_FILENAME("out.");
static const int RAM_SIZE_WORDS = 4096;

struct Options {
	bool verbose = false;
	std::string filename;
	std::string output_filename = DEFAULT_OUTPUT_FILENAME;
};

static Options opt;

void parse_args(int arc, char** argv) {
	std::vector<std::string> args;

	for (int i = 0; i < arc; ++i) {
		args.emplace_back(argv[i]);
	}

	// Start at one to ignore filename of "this"
	// ex: cc.ext -v filename.c
	for (int i = 1; i < static_cast<int>(args.size()); ++i) {
		const auto& arg = args.at(i);

		if ("-v" == arg || "--verbose" == arg) {
			opt.verbose = true;
		} else if ("-o" == arg) {
			opt.output_filename = args.at(++i) + ".";
		} else {
			// if it's none of the options, assume filename
			opt.filename = arg;
		}
	}
}

int main(int argc, char **argv) {
	parse_args(argc, argv);
	if (opt.filename == "") {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	try {
		FileReader f;

		auto ret = compile_tu(opt.filename, f);
		write_file(opt.output_filename + "s", ret.assembly);


		write_file(opt.output_filename + "hex", machine_inst_to_hex(ret.machine_code));
		write_bin_file(opt.output_filename + "bin", ret.machine_code);
		auto srec = machine_inst_to_srec(ret.machine_code, ret.load_address);
		write_file(opt.output_filename + "srec", srec);

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
