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
		{"loadc rc 32 \n .top: \n loadc rc 31 \n jmp .top", "A820 A81F 9301 "},
		{"jmp 1", "9301 "},
		{"jmp 0", "9300 "},
		{"", ""},
		{" ", ""},
		{"# test comment", ""},
	};

	for (const auto& test_point : test_points) {
		const std::string output = assemble(test_point.input);
		
		INFO(test_point.input);
		CHECK(output == test_point.expected_out);
	}
}

TEST_CASE("Test assembler should throw", "[asm]") {
	std::vector<std::string> test_points = {
		"jmp -1", // constant jump location must be 0 .. 255
		"jmp", // missing argument
		"add ra, ra", // missing argument
		"add ra, 32", // missing argument (constant)
		"add ra, ra, rb, rc", // too many arguments
		"add ra, ra, rb, 23", // too many arguments (constant)
		"add ra, 32, rb", // invalid constant location
		"add ra, rb, rb", // input 1 cannot be rb (only ra, rc)
	};

	for (const auto& test_point : test_points) {
		INFO(test_point);
		CHECK_THROWS(assemble(test_point));
	}
}

TEST_CASE("Test assembler programs", "[asm]") {
	std::vector<Test_Point> test_points = {
		{"flasher_program.txt", "A055 A4FF B000 4000 9302 "},
		{"fib_program.txt", "A001 A401 0800 8100 8600 B000 9302 "},
		{"label.txt", "A00A 0101 B000 9301 "},
	};

	for (const auto& test_point : test_points) {
		std::string program = file_reader(std::string("./cr-cc-test/test_data/") + test_point.input);

		REQUIRE(program.length() != 0);

		std::string output = assemble(program);

		INFO(test_point.expected_out);
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
