#include "cmdline_parser.h"

#include <iostream>


const std::string Compiler_Options::DEFAULT_OUTPUT_FILENAME = "out.";

Compiler_Options parse_args(int arc, char** argv) {
	std::vector<std::string> args;

	for (int i = 0; i < arc; ++i) {
		args.emplace_back(argv[i]);
	}

	if (args.size() == 1) {
		args.emplace_back("-h");
	}

	Compiler_Options opt;

	// Start at one to ignore filename of "this"
	// ex: cc.ext -v filename.c
	for (int i = 1; i < static_cast<int>(args.size()); ++i) {
		const auto& arg = args.at(i);

		if ("-h" == arg || "--help" == arg) {
			std::cout <<
				"cr-cc - Compiler for CR-CPU architecture\n"
				"\n"
				"Usage: cr-cc [options] file..\n"
				"\n"
				"Options:\n"
				"  -v, --verbose         Enable verbose output\n"
				"  -o <filename>         Outputs to a particular filename\n"
				"  -c                    Only compiles the file(s) to objects, does not link\n"
				"  --lib                 Output as a library\n"
				"  -S                    Also output the assembly\n"
				"  --sim                 Simulates the program on an emulator\n"
				"  --no-main             When linking, does not include the default jump to main()\n"
				"  --link-addr <number>  Relocates the program to run at the given address\n"
				"  --map                 Also output in map format\n"
				"  -I<dir>               Add dir to the list of paths searched for includes\n"
				<< std::endl;

			opt.should_exit = true;
			return opt;
		} else if ("-v" == arg || "--verbose" == arg) {
			opt.verbose = true;
		} else if ("-o" == arg) {
			opt.output_filename = args.at(++i) + ".";
		} else if ("-c" == arg) {
			opt.compile_only = true;
		} else if ("--sim" == arg) {
			opt.should_sim = true;
		} else if ("--no-main" == arg) {
			opt.include_main = false;
		} else if ("--link-addr" == arg) {
			opt.link_address = std::stoi(args.at(++i), nullptr, 0);
		} else if ("--map" == arg) {
			opt.output_map = true;
		} else if ("-S" == arg) {
			opt.output_assembly = true;
		} else if ("--lib" == arg) {
			opt.output_lib = true;
			opt.include_main = false;
		} else if (arg.at(0) == '-' && arg.at(1) == 'I') {
			auto path = arg.substr(2);
			opt.include_paths.push_back(path);
		} else {
			// if it's none of the options, assume filename
			opt.filenames.push_back(arg);
		}
	}

	return opt;
}