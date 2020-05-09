#include "compiler.h"
#include "file_io.h"
#include "utilities.h"
#include "simulator.h"
#include "linker.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <locale>
#include <cstdlib>

static const std::string DEFAULT_OUTPUT_FILENAME("out.");
static const int RAM_SIZE_WORDS = 4096;
static const char* STDLIB_ENV_VAR = "CRSTDLIBPATH";

struct Options {
	bool verbose = false;
	bool should_sim = false;
	bool compile_only = false;
	std::vector<std::string> filenames;
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
		} else if ("-c" == arg) {
			opt.compile_only = true;
		} else if ("--sim" == arg) {
			opt.should_sim = true;
		} else {
			// if it's none of the options, assume filename
			opt.filenames.push_back(arg);
		}
	}
}

// Stupid workaround just to get thousands separator to print
struct Thousand_Sep : public std::numpunct<char> {
	char do_thousands_sep() const override {
		return ',';
	}
	std::string do_grouping() const override {
		return "\3";
	}
};

int main(int argc, char **argv) {
	parse_args(argc, argv);
	if (opt.filenames.size() == 0) {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	try {
		FileReader f;
		// FileReader defaults to including the current directory
		// Next add all user defined directories
		{
			// TODO
		}
		// Finally add stdlib path last so it's checked last
		{
			auto stdlib_path = std::getenv(STDLIB_ENV_VAR);
			if (stdlib_path) {
				f.add_directory(std::string(stdlib_path));
			} else {
				throw std::logic_error(std::string("Could not find stdlib path: ") + STDLIB_ENV_VAR);
			}
		}

		std::vector<Object::Object_Container> objs;

		if (!opt.compile_only) {
			// If we're compiling only, we just want to create objs.
			// For the normal case, we also want to link into an exe.
			auto ret = compile_tu("main.s", f);
			objs.push_back(ret.item);
		}

		for (const auto& filename : opt.filenames) {
			auto ret = compile_tu(filename, f);
			objs.push_back(ret.item);
			write_file(opt.output_filename + "s", ret.assembly);
		}

		// TODO if compile_only then don't link
		auto exe = link(std::move(objs));

		const auto& machine_code = std::get<Object::Executable>(exe.contents).machine_code;

		write_file(opt.output_filename + "hex", machine_inst_to_hex(machine_code));
		write_bin_file(opt.output_filename + "bin", machine_code);
		auto srec = machine_inst_to_srec(machine_code, exe.load_address);
		write_file(opt.output_filename + "srec", srec);

		if (opt.verbose) {
			std::cout << "Code size: " << machine_code.size() << "/" << RAM_SIZE_WORDS
				<< " (" << std::fixed << std::setprecision(2)
				<< static_cast<float>(machine_code.size()) / RAM_SIZE_WORDS * 100 << "%)"
				<< std::endl;

			std::cout << "\n" << srec;
		}

		if (opt.should_sim) {
			auto program_loader = compile_tu("program_loader.s", f);

			Simulator sim;
			sim.load(0, std::get<Object::Object_Type>(program_loader.item.contents).machine_code);
			sim.load(exe.load_address, machine_code);

			const int total_steps = 16000000;// up to one second of operation
			sim.run_until_halted(total_steps);

			std::cout << "Sim result: 0x" << std::hex << sim.get_state().ra
				<< " (" << std::dec << sim.get_state().ra << ")" << std::endl;
			std::cout.imbue(std::locale(std::locale(), new Thousand_Sep));
			std::cout << "Is halted: " << std::boolalpha << sim.get_state().is_halted << " steps used: " << (total_steps - sim.get_state().steps_remaining) << std::endl;
		}

	} catch (std::logic_error& e) {
		std::cout << "ASSEMBLY ERROR: \n";
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
