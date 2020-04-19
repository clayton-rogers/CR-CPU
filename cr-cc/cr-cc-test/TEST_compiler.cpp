#include "c_to_asm.h"
#include "file_io.h"
#include "compiler.h"
#include "utilities.h"
#include "simulator.h"

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
		if (item == "test_data/valid_c/program_loader.s") { continue; }
		FileReader fr;
		fr.add_directory(DIR);

		// Catching the exception in the test allows us to continue
		// rather than stopping at the first failed compile
		try {

			auto ret = compile_tu(item, fr);
			auto program_loader = compile_tu("program_loader.s", fr);

			// Load the compiled code into the simulator and see that the return is correct
			Simulator sim;
			sim.load(ret.load_address, ret.machine_code);
			sim.load(program_loader.load_address, program_loader.machine_code);

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
