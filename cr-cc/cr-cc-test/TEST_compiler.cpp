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
		fr.add_directory(DIR);

		// Catching the exception in the test allows us to continue
		// rather than stopping at the first failed compile
		try {

			auto test_program = compile_tu(item, fr);
			auto init_main = compile_tu("./stdlib/main.s", fr); // this is what calls the main fn
			auto program_loader = compile_tu("./stdlib/program_loader.s", fr); // This jumps to 0x200

			std::vector<Object::Object_Container> objs;
			objs.push_back(init_main);
			objs.push_back(test_program);
			auto exe = link(std::move(objs), 0x200);

			// Load the compiled code into the simulator and see that the return is correct
			Simulator sim;
			sim.load(exe.load_address, std::get<Object::Executable>(exe.contents).machine_code);
			sim.load(0, std::get<Object::Object_Type>(program_loader.contents).machine_code);

			sim.run_until_halted(20000);
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

	auto program_loader = compile_tu("program_loader.s", fr);

	Simulator sim;
	sim.load(exe.load_address, std::get<Object::Executable>(exe.contents).machine_code);
	sim.load(0, std::get<Object::Object_Type>(program_loader.contents).machine_code);

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
