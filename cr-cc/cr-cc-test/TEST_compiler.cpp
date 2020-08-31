#include "c_to_asm.h"
#include "file_io.h"
#include "compiler.h"
#include "utilities.h"
#include "simulator.h"
#include "linker.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>
#include <iostream>

TEST_CASE("Test basic function of compiler", "[c]") {
	FileReader fr;
	fr.add_directory("./test_data");
	
	auto ret = compile_tu("first.c", fr);
}

TEST_CASE("Compiler Benchmarks", "[.][bench]") {
	FileReader fr;
	fr.add_directory("./test_data");

	std::string filename = "first.c";

	BENCHMARK(std::string("File: ") + filename) {
		return c_to_asm(filename, fr);
	};
}

static int get_expected_return(std::string filename) {
	std::string file_contents = read_file(filename);

	// For each file we expect the following line to be at the top
	// "// ret: <number>
	const std::string MAGIC_STR = "ret:";
	auto location = file_contents.find(MAGIC_STR);
	if (location == std::string::npos) {
		throw std::logic_error("Could not find magic string in test file");
	}
	location += MAGIC_STR.size();
	// allow any spaces between : and number
	while (file_contents.at(location) == ' ') {
		++location;
	}
	return std::stoi(file_contents.substr(location), nullptr, 0); // get the number in any base
}

TEST_CASE("Exaustive test of Compiler", "[c]") {
	
	// These test programs were mostly stolen and slightly modified from:
	// https://github.com/nlsandler/write_a_c_compiler.git
	const std::string DIR = "test_data/valid_c/";
	auto dir_list = read_directory(DIR);

	for (const auto& item : dir_list) {
		INFO(item);
		FileReader fr;
		fr.add_directory("./stdlib/");
		fr.add_directory(DIR);

		// Catching the exception in the test allows us to continue
		// rather than stopping at the first failed compile
		try {

			auto test_program = compile_tu(item, fr);
			auto init_main = compile_tu("./stdlib/main.s", fr); // this is what calls the main fn
			auto program_loader_o = compile_tu("./stdlib/program_loader.s", fr); // This jumps to 0x200
			auto stream = read_bin_file("./stdlib/stdlib.a");
			auto stdlib = Object::Object_Container::from_stream(stream);

			std::vector<Object::Object_Container> objs;
			objs.push_back(init_main);
			objs.push_back(test_program);
			objs.push_back(stdlib); // link stdlib for those couple that need it
			auto exe = link(std::move(objs), 0x200);

			auto program_loader_exe = link({ program_loader_o }, 0);

			// Load the compiled code into the simulator and see that the return is correct
			Simulator sim;
			sim.load(exe);
			sim.load(program_loader_exe);

			sim.run_until_halted(55000); // longest test is currently isqrt.c
			CHECK(sim.get_state().is_halted == true);

			// Check that the program produced the desired result
			const int actual_program_output = sim.get_state().ra;
			const int expected_program_output = get_expected_return(item);

			CHECK(actual_program_output == expected_program_output);

		} catch (const std::exception & e) {
			std::string message("Unexpected exception with message:\n");
			message = message + e.what();
			INFO(message);
			CHECK(false);
			continue;
		}
	}
}

// We don't actually want to burden the auto build with this

//TEST_CASE("Exaustive Compiler Benchmarks", "[.][bench]") {
//
//	const std::string DIR = "test_data/valid_c/";
//	auto dir_list = read_directory(DIR);
//
//	for (const auto& item : dir_list) {
//		if (item == "test_data/valid_c/program_loader.s") { continue; }
//		FileReader fr;
//		fr.add_directory(DIR);
//
//		BENCHMARK(std::string("Filename: " ) + item) {
//			return compile_tu(item, fr);
//		};
//	}
//}

TEST_CASE("Invalid C programs", "[c]") {

	const std::string DIR = "test_data/invalid_c/";
	auto dir_list = read_directory(DIR);

	for (const auto& item : dir_list) {
		INFO(item);
		FileReader fr;
		fr.add_directory(DIR);

		CHECK_THROWS(compile_tu(item, fr));
	}
}

TEST_CASE("Whole C program", "[c]") {
	FileReader fr;
	
	// For the actual program
	fr.add_directory("./test_data/whole_program/");
	// For the sub lib
	fr.add_directory("./test_data/whole_program/sub");
	// For main.s
	fr.add_directory("./stdlib/");

	std::vector<Object::Object_Container> objs;
	objs.push_back(compile_tu("main.s", fr));
	objs.push_back(compile_tu("main.c", fr));
	objs.push_back(compile_tu("add.c", fr));
	objs.push_back(compile_tu("sub.c", fr));

	auto exe = link(std::move(objs), 0x200);

	auto program_loader_o = compile_tu("program_loader.s", fr);
	auto program_loader_exe = link({ program_loader_o }, 0);

	Simulator sim;
	sim.load(exe);
	sim.load(program_loader_exe);

	sim.run_until_halted(20000);
	CHECK(sim.get_state().is_halted == true);

	// Check that the program produced the desired result
	const int actual_program_output = sim.get_state().ra;
	const int expected_program_output = 77;

	CHECK(actual_program_output == expected_program_output);
}

TEST_CASE("Test file io filename methods", "[c]") {
	std::string filename("test.txt");

	CHECK(get_base_filename(filename) == std::string("test"));
	CHECK(get_file_extension(filename) == std::string("txt"));
}

TEST_CASE("Linker with multi segment files", "[link]") {
	// These two relocation files are specially crafted so that addresses
	// that need relocation span a page boundary.
	//
	// Ex.
	// Originally the reference was to 0x0006
	// The relocation offset is 0x00FE
	//
	// If you handle the two bytes separately you get:
	// 00 + 00 =  00 high byte
	// 06 + FE = 104 low byte
	// Thus giving 0x0004 when it should be 0x0104
	// Issue was cause by the overflow of the low byte not incrementing the high byte

	using namespace Object;

	FileReader f;
	f.add_directory("./test_data/relocation/");
	f.add_directory("./stdlib/"); // for main.s


	std::vector<Object_Container> objs;
	objs.push_back(compile_tu("main.s", f));
	objs.emplace_back(compile_tu("relocation_asm1.s", f));
	objs.emplace_back(compile_tu("relocation_asm2.s", f));

	auto exe = link(std::move(objs), 0x200);

	auto program_loader_o = compile_tu("program_loader.s", f);
	auto program_loader_exe = link({ program_loader_o }, 0);

	Simulator sim;
	sim.load(exe);
	sim.load(program_loader_exe);

	sim.run_until_halted(20000);
	CHECK(sim.get_state().is_halted == true);
	CHECK(sim.get_state().ra == 0x0000);
}
