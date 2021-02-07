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
#include <algorithm>


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
		auto assembly = read_file(filename);
		return assemble(assembly);
	} else if (file_extension == "o" || file_extension == "map" || file_extension == "a") {
		const auto stream = read_bin_file(filename);
		return Object::Object_Container::from_stream(stream);
	} else {
		throw std::logic_error("Cannot handle file with extension: " + file_extension);
	}

}

static void print_function_names(const Object::Object_Container& obj) {

	struct Symbol {
		int offset;
		std::string name;
	};

	std::vector<Symbol> symbols;

	// Add all symbols to the map, sorting them
	const auto& exported_symbols = std::get<Object::Object_Type>(obj.contents).exported_symbols;
	if (exported_symbols.size() == 0) { return; }
	for (const auto& symb : exported_symbols) {
		const auto type = symb.type;
		if (type == Object::Symbol_Type::FUNCTION) {
			symbols.push_back({ symb.offset, symb.name });
		} else if (type == Object::Symbol_Type::DATA) {
			symbols.push_back({ symb.offset, " Static Data" });
		}
	}

	// Sort them by name
	std::sort(symbols.begin(), symbols.end(),
		[](Symbol a, Symbol b) {
			return a.name < b.name;
		});

	// Output the size of the symbols
	// Starting at the symbol localtion, look for the next ret instruction or end of code
	// This assumes that all functions will only have a single ret.
	const auto& machine_code = std::get<Object::Object_Type>(obj.contents).machine_code;
	std::uint16_t ret_opcode = 0xC100;
	int total = 0;
	std::cout << std::endl;
	for (const auto& symb : symbols) {
		int ptr = symb.offset;
		while (ptr < static_cast<int>(machine_code.size()) && machine_code.at(ptr) != ret_opcode) {
			++ptr;
		}

		const int size = (ptr - symb.offset);
		total += size;
		std::cout << symb.name << " - " << size << " words" << std::endl;
	}
	std::cout << " Total: " << total << std::endl;
}

void handle_exe(const Object::Object_Container& exe, Compiler_Options opt) {

	const auto& machine_code = std::get<Object::Executable>(exe.contents).machine_code;

	if (opt.output_map) {
		const auto map = to_map(exe);
		const auto map_stream = map.to_stream();
		const auto map_filename = opt.output_filename + ".map";
		write_bin_file(map_filename, map_stream);
	}
	if (opt.output_hex) {
		write_file(opt.output_filename + ".hex", machine_inst_to_hex(machine_code));
	}
	if (opt.output_srec) {
		auto srec = exe_to_srec(exe);
		write_file(opt.output_filename + ".srec", srec);

		if (opt.output_srec_stdout) {
			std::cout << "\n" << srec << std::endl;
		}
	}

	if (opt.verbose) {
		std::cout << "Code size: " << machine_code.size() << "/" << RAM_SIZE_WORDS
			<< " (" << std::fixed << std::setprecision(2)
			<< static_cast<float>(machine_code.size()) / RAM_SIZE_WORDS * 100 << "%)"
			<< std::endl;
	}

	if (opt.should_sim) {

		Simulator sim;
		sim.load(exe);
		sim.run_until_halted(opt.sim_steps);

		std::cout << "Sim result: 0x" << std::hex << sim.get_state().ra
			<< " (" << std::dec << sim.get_state().ra << ")" << std::endl;
		std::cout << "PC: 0x" << std::hex << sim.get_state().pc
			<< " (" << std::dec << sim.get_state().pc << ")" << std::endl;
		std::cout.imbue(std::locale(std::locale(), new Thousand_Sep));
		std::cout << "Is halted: " << std::boolalpha << sim.get_state().is_halted << " steps used: " << (opt.sim_steps - sim.get_state().steps_remaining) << std::endl;
	}
}

int compile(Compiler_Options opt) {
	if (opt.should_exit) {
		return 1;
	}
	if (opt.filenames.size() == 0) {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	std::string stdlib_path = ".";
	if (opt.include_stdlib) {
		auto stdlib_path_c = std::getenv(STDLIB_ENV_VAR);
		if (stdlib_path_c) {
			stdlib_path = std::string(stdlib_path_c);
		} else {
			throw std::logic_error(std::string("Could not find stdlib path: ") + STDLIB_ENV_VAR);
		}
	}

	FileReader f(stdlib_path);
	// FileReader defaults to including the current directory
	// Next add all user defined directories
	for (const auto& include_path : opt.include_paths) {
		f.add_directory(include_path);
	}


	if (opt.filenames.size() == 1 &&
		get_file_extension(opt.filenames.at(0)) == "bin") {

		const auto stream = read_bin_file(opt.filenames.at(0));
		auto exe = Object::Object_Container::from_stream(stream);

		handle_exe(exe, opt);

		return 0;
	}

	// If the input is not a bin file, then assume this is actually a file(s) that need compiling/link
	try {
		std::vector<Object::Object_Container> objs;

		if (opt.include_main) {
			auto assembly = read_file(stdlib_path + "/main.s");
			auto container = assemble(assembly);
			objs.push_back(container);
		}

		for (const auto& filename : opt.filenames) {

			if (get_file_extension(filename) == "c" && opt.output_assembly) {
				const auto assembly = c_to_asm(filename, f);
				const auto asm_filename = get_base_filename(filename) + ".s";
				write_file(asm_filename, assembly);
			}

			const auto container = compile_tu(filename, f);
			objs.push_back(container);

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

		if (opt.function_size) {
			for (const auto& obj : objs) {
				print_function_names(obj);
			}
		}

		if (opt.compile_only) {
			return 0;
		}

		if (opt.output_lib) {
			auto lib = make_lib(objs);
			auto stream = lib.to_stream();
			write_bin_file(opt.output_filename, stream);
			return 0;
		}

		// Automatically link with stdlib
		{
			auto stream = read_bin_file(stdlib_path + "/stdlib.a");
			auto stdlib = Object::Object_Container::from_stream(stream);
			objs.push_back(stdlib);
		}

		auto exe = link(std::move(objs), opt.link_address);
		{
			auto stream = exe.to_stream();
			write_bin_file(opt.output_filename + ".bin", stream);
		}

		// Handle any outputs and simulation
		handle_exe(exe, opt);

	} catch (const std::logic_error& e) {
		// TODO actually separate out the types of errors
		std::cout << "COMPILER ERROR: \n";
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}
