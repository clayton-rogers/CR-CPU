#include "compiler.h"

#include "c_to_asm.h"
#include "assembler.h"
#include "file_io.h"
#include "utilities.h"
#include "simulator.h"
#include "linker.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <locale>
#include <stdexcept>


static const int RAM_SIZE_WORDS = 4096;
static const char* STDLIB_ENV_VAR = "CRSTDLIBPATH";


// Stupid workaround just to get thousands separator to print
struct Thousand_Sep : public std::numpunct<char> {
	char do_thousands_sep() const override {
		return ',';
	}
	std::string do_grouping() const override {
		return "\3";
	}
};

Object::Object_Container compile_tu(std::string filename, FileReader f) {

	std::string file_extension = get_file_extension(filename);

	if (file_extension == "c") {
		// TODO if any include paths are specified add them here.
		auto assembly = c_to_asm(filename, f);
		return assemble(assembly);
	} else if (file_extension == "s") {
		auto assembly = f.read_file_from_directories(filename);
		return assemble(assembly);
	} else if (file_extension == "o" || file_extension == "map") {
		const auto stream = f.read_bin_file_from_directories(filename);
		return Object::Object_Container::from_stream(stream);
	} else {
		throw std::logic_error("Cannot handle file with extension: " + file_extension);
	}

}

static Object::Object_Container to_map(const Object::Object_Container& obj) {
	auto object = std::get<Object::Executable>(obj.contents);

	Object::Map map_contents;
	map_contents.exported_symbols = object.exported_symbols;

	Object::Object_Container map;
	map.contents = map_contents;
	map.load_address = obj.load_address;

	return map;
}

int compile(Compiler_Options opt) {
	if (opt.should_exit) {
		return 1;
	}
	if (opt.filenames.size() == 0) {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	try {
		FileReader f;
		// FileReader defaults to including the current directory
		// Next add all user defined directories
		for (const auto& include_path : opt.include_paths) {
			f.add_directory(include_path);
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

		if (opt.include_main) {
			auto container = compile_tu("main.s", f);
			objs.push_back(container);
		}

		for (const auto& filename : opt.filenames) {
			const auto container = compile_tu(filename, f);
			objs.push_back(container);

			if (get_file_extension(filename) == "c" && opt.output_assembly) {
				const auto assembly = c_to_asm(filename, f);
				const auto asm_filename = get_base_filename(filename) + ".s";
				write_file(asm_filename, assembly);
			}

			if (opt.compile_only) {
				const auto output_filename = get_base_filename(filename) + ".o";
				const auto stream = container.to_stream();
				write_bin_file(output_filename, stream);

				if (opt.verbose) {
					const auto object = std::get<Object::Object_Type>(container.contents);
					std::cout << "Code size: " << object.machine_code.size() << std::endl;
				}
			}
		}

		if (opt.compile_only) {
			return 0;
		}

		auto exe = link(std::move(objs), opt.link_address);

		const auto& machine_code = std::get<Object::Executable>(exe.contents).machine_code;

		{
			auto stream = exe.to_stream();
			write_bin_file(opt.output_filename + "bin", stream);
		}
		if (opt.output_map) {
			const auto map = to_map(exe);
			const auto map_stream = map.to_stream();
			const auto map_filename = opt.output_filename + "map";
			write_bin_file(map_filename, map_stream);
		}
		write_file(opt.output_filename + "hex", machine_inst_to_hex(machine_code));
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
			sim.load(0, std::get<Object::Object_Type>(program_loader.contents).machine_code);
			sim.load(exe.load_address, machine_code);

			const int total_steps = 16000000;// up to one second of operation
			sim.run_until_halted(total_steps);

			std::cout << "Sim result: 0x" << std::hex << sim.get_state().ra
				<< " (" << std::dec << sim.get_state().ra << ")" << std::endl;
			std::cout.imbue(std::locale(std::locale(), new Thousand_Sep));
			std::cout << "Is halted: " << std::boolalpha << sim.get_state().is_halted << " steps used: " << (total_steps - sim.get_state().steps_remaining) << std::endl;
		}

	} catch (const std::logic_error& e) {
		// TODO actually separate out the types of errors
		std::cout << "COMPILER ERROR: \n";
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
