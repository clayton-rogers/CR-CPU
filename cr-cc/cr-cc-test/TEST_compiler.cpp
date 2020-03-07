#include "c_to_asm.h"
#include "file_io.h"
#include "assembler.h"
#include "compiler.h"
#include "utilities.h"

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

TEST_CASE("Exaustive test of Compiler", "[c]") {
	
	auto dir_list = read_directory("test_data/");

	for (const auto& item : dir_list) {
		std::cout << item << std::endl;
	}
}
