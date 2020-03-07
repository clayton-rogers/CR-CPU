#include "c_to_asm.h"
#include "file_io.h"
#include "assembler.h"
#include "compiler.h"
#include "utilities.h"
#include "simulator.h"
#include "simulator_ram.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>
#include <iostream>

TEST_CASE("Test basic function of compiler", "[c]") {
	FileReader fr;
	fr.add_directory("./test_data");
	
	auto ret = compile_tu("first.c", fr);

	//std::string test_filename = "basic.c";
	//std::string result = compile(test_filename, fr);
	//std::cout << result << std::endl;

	//std::string test_filename = "first.c";
	//std::string result = c_to_asm(test_filename, fr);
	//std::cout << "\n\n" << result << std::endl;
	//std::cout << result << std::endl;
	//std::uint16_t offset;
	//auto machine_code = assemble(result, &offset);
	//auto m_code_hex = machine_inst_to_srec(machine_code, offset);
	//std::cout << "\n\n" << m_code_hex << std::endl;
}

TEST_CASE("Test properties of cast uint16", "[c]") {

	SECTION("At pos limit") {
		int a = 0x7FFF;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0x7FFF);
	}
	SECTION("Above limit") {
		int a = 0x8000;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0x8000);
	}
	SECTION("At pos max") {
		int a = 0xFFFF;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0xFFFF);
	}
	SECTION("Negative") {
		int a = -1;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0xFFFF);
	}
	SECTION("At neg max") {
		int a = -32768;
		std::uint16_t b = static_cast<std::uint16_t>(a);

		CHECK(b == 0x8000);
	}
}

TEST_CASE("Compiler Benchmarks", "[bench]") {
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
		if (item == "test_data/valid_c/program_loader.s") { continue; }
		FileReader fr;
		fr.add_directory(DIR);

		auto ret = compile_tu(item, fr);
		auto program_loader = compile_tu("program_loader.s", fr);

		// Load the compiled code into the simulator and see that the return is correct
		auto bus = std::make_shared<Simulator_Bus>();
		Simulator_Ram ram(bus);
		ram.load_ram(ret.load_address, ret.machine_code);
		ram.load_ram(program_loader.load_address, program_loader.machine_code);
		Simulator sim(bus);

		int step = 0;
		const int MAX_STEPS = 2000;
		while (!sim.is_halted && step < MAX_STEPS) {
			sim.step();
			ram.step();
			++step;
		}
		CHECK(step != MAX_STEPS); // If program is infinite loop

		// Check that the program produced the desired result
		const int actual_program_output = sim.get_ra();
		const int expected_program_output = get_expected_return(item);

		CHECK(actual_program_output == expected_program_output);
	}
}

TEST_CASE("Invalid C programs", "[c]") {

	const std::string DIR = "test_data/invalid_c/";
	auto dir_list = read_directory(DIR);

	for (const auto& item : dir_list) {
		FileReader fr;
		fr.add_directory(DIR);

		CHECK_THROWS(compile_tu(item, fr));
	}
}
