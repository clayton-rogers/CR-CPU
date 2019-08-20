#include "assembler.h"
#include "file_io.h"
#include "machine_io.h"
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
		{"loadc rc 32 \n .top: \n loadc rc 31 \n jmp .top", "A820 A81F 93FF "}, // jmp -1
		{"loadch rc 0xEE", "AAEE "},
		{"jmp .end \n .end: \n nop", "9301 F000 "}, // jmp 1
		{"jmp 1", "9301 "},
		{"jmp 0", "9300 "},
		{"", ""},
		{" ", ""},
		{"# test comment", ""},
		{"xor ra, ra, rb # test comment", "4000 "},
		{"add ra, ra, rb", "0000 "},
		{"sub ra, ra, rb", "1000 "},
		{"and ra, ra, rb", "2000 "},
		{"or ra, ra, rb",  "3000 "},
		{"xor ra, ra, rb", "4000 "},
		{"shftl rb, rb", "5500 "},
		{"shftl rb, 0x16", "5716 "},
		{"shftr rc, 0xA1", "5AA1 "},
		{"load rd, 10", "6F0A "},
		{"load ra, rb, 0x10", "6110 "},
		{"store rd, 10", "7F0A "},
		{"store ra, rb, 0x10", "7110 "},
		{"jmp ra, 3", "9003 "},
		{"jmp 3", "9303 "},
		{"jifz 0x10", "9710 "},
		{"jinz 0x20", "9B20 "},
		{"jgz  rc, 0xff", "9EFF "},
		{"out ra", "B000 "},
		{"out rd", "BC00 "},
		{"halt", "E000 "},
		{"nop", "F000 "},
		{"jmp -1", "93FF "},
		{".var int[3] abc \n loadc ra, 10 \n store ra, .abc[2]","A00A 7302 "},
	};

	for (const auto& test_point : test_points) {
		INFO(test_point.input);

		std::string output = machine_inst_to_unformatted(assemble(test_point.input));

		CHECK(output == test_point.expected_out);
	}
}

TEST_CASE("Test assembler should throw", "[asm]") {
	std::vector<std::string> test_points = {
		"jmp", // missing argument
		"add ra, ra", // missing argument
		"add ra, 32", // missing argument (constant)
		"add ra, ra, rb, rc", // too many arguments
		"add ra, ra, rb, 23", // too many arguments (constant)
		"add ra, 32, rb", // invalid constant location
		"add ra, rb, rb", // input 1 cannot be rb (only ra, rc)
		"add ra, ra, ra", // input 2 cannot be ra (only rb, const)
		"jinz rd, 0x10", // offset register cannot be rd (only ra, rb, rc)
		"jinz 0x10, 0x10", // only one constant allowed
		"out 0x10", // missing source register
		"out", // missing source register
		"out ra, 0x10", // too many args
		"halt rb", // does not allow registers
		"halt 0x10", // does not allow constants
		"nop rb", // does not allow registers
		"nop 0x10", // does not allow constants
		"nop \n nop \n jmp .top", // jump to label that doesn't exist
		".top: \n nop \n .top: \n nop \n jmp .top", // cannot define duplicate label
	};

	for (const auto& test_point : test_points) {
		INFO(test_point);
		CHECK_THROWS(assemble(test_point));
	}
}

static const std::vector<Test_Point> test_programs = {
	{"flasher_program.txt", "A055 A4FF B000 4000 9302 "},
	{"fib_program.txt", "A001 A401 0800 8100 8600 B000 9302 "},
	{"label.txt", "A00A 0101 B000 93FE "},
	// Note this program should produce the output 003C when run
	{"sum.txt", "A00A 7300 A00B 7301 A017 7302 A008 7303 A003 7304 A804 6200 0101 7200 1B01 8200 9FFB A804 A400 8100 6600 0000 1B01 8400 8200 9FFA B400 E000 " },
	// Should produce 0x0016 output
	{"var_test.txt", "A00A 7300 A00B A401 0000 7301 6300 6701 0000 7300 B000 E000 "},
	// Should produce 0x002B output
	{"array_test.txt", "A00A 7300 A00B 7301 A016 7302 A000 6702 0400 A801 6200 0400 1B01 8200 9FFC B400 E000 "},
	{"flash.txt", "A0FF B000 A090 A201 1101 9FFF A000 B000 A090 A201 1101 9FFF 93F4 "},
};

TEST_CASE("Test assembler programs", "[asm]") {
	for (const auto& test_point : test_programs) {
		INFO(test_point.input);

		std::string program = read_file(std::string("./test_data/") + test_point.input);

		REQUIRE(program.length() != 0);

		std::string output = machine_inst_to_unformatted(assemble(program));

		CHECK(output == test_point.expected_out);
	}
}

TEST_CASE("Benchmarks", "[bench]") {
	for (const auto& test_point : test_programs) {
		std::string program = read_file(std::string("./test_data/") + test_point.input);

		REQUIRE(program.length() != 0);

		BENCHMARK(std::string("Benchmark: ") + test_point.input) {
			return assemble(program);
		};
	}
}
