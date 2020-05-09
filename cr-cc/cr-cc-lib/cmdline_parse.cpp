#include "cmdline_parser.h"


const std::string Compiler_Options::DEFAULT_OUTPUT_FILENAME = "out.";

Compiler_Options parse_args(int arc, char** argv) {
	std::vector<std::string> args;

	for (int i = 0; i < arc; ++i) {
		args.emplace_back(argv[i]);
	}

	Compiler_Options opt;

	// Start at one to ignore filename of "this"
	// ex: cc.ext -v filename.c
	for (int i = 1; i < static_cast<int>(args.size()); ++i) {
		const auto& arg = args.at(i);

		if ("-v" == arg || "--verbose" == arg) {
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
		} else {
			// if it's none of the options, assume filename
			opt.filenames.push_back(arg);
		}
	}

	return opt;
}