#include "assembler.h"
#include "file_reader.h"
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>

struct Test_Point {
	std::string input;
	std::string expected_out;
};

TEST_CASE("Test assembler instructions", "[asm]") {
	std::vector<Test_Point> test_points = {
		{"ADD RA, RC, 10", "030A "},
		{"loadc ra, 0x30", "A030 "},
		{"loadc rb, 0xFF", "A4FF "},
		{"loadc rc 32",    "A820 "},
		{"", ""},
		{"# test comment", ""},
	};

	for (const auto& test_point : test_points) {
		const std::string output = assemble(test_point.input);

		CHECK(output == test_point.expected_out);
	}
}

TEST_CASE("Test assembler programs", "[asm]") {
	std::vector<Test_Point> test_points = {
		{"flasher_program.txt", "A055 A4FF B000 4000 9302 "},
		{"fib_program.txt", "A001 A401 0800 8100 8600 B000 9302 "},
	};

	for (const auto& test_point : test_points) {
		std::string program = file_reader(std::string("./cr-cc-test/test_data/") + test_point.input);

		REQUIRE(program.length() != 0);

		std::string output = assemble(program);

		CHECK(output == test_point.expected_out);
	}
}

TEST_CASE("Benchmarks", "[bench]") {
	std::string program = file_reader("./cr-cc-test/test_data/bench_program1.txt");

	REQUIRE(program.length() != 0);

	BENCHMARK("Assemble test program") {	
		return assemble(program);
	};
}
