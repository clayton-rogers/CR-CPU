#include "assembler.h"
#include "file_io.h"
#include "machine_io.h"
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>

TEST_CASE("Internal assembler test", "[asm]") {
	// Don't bother checking anything else if the internal test fails
	REQUIRE(assembler_internal_test() == true);
}

struct Test_Point {
	std::string input;
	std::string expected_out;
};

TEST_CASE("Test assembler instructions", "[asm]") {
	std::vector<Test_Point> test_points = {
		{"", ""},
		{" ", ""},
		{"	", ""},
		{" # test comment", ""},
		{"xor ra, rb # test comment", "4100 "},
		{"ADD RA, 10", "030A "},
		{"add ra, ra", "0000 "},
		{"sub ra, rb", "1100 "},
		{"and ra, bp", "2200 "},
		{"or ra, ra",  "3000 "},
		{"xor ra, ra", "4000 "},
		{"shftl rb, rb", "5500 "},
		{"shftl rb, 0x16", "5716 "},
		{"shftr bp, 0xA1", "5AA1 "},
		{"load sp, 10", "6E0A "},
		{"load ra, 0x10", "6210 "},
		{"load.bp rb, 10", "640A "},
		{"load.sp ra, -5", "61FB "},
		{"store sp, 10", "7E0A "},
		{"store ra, 0x10", "7210 "},
		{"store.bp rb, 10", "740A "},
		{"store.sp ra, -5", "71FB "},
		{"store sp, 10", "7E0A "},
		{"mov ra, rb", "8100 "},
		{"mov ra, sp", "8300 "},
		{"mov bp, sp", "8B00 "},
		{"jmp .end \n .end: \n nop", "9201 F000 "}, // jmp abs 1
		{"nop \n jmp.r .end \n nop \n .end: \n nop", "F000 9002 F000 F000 "}, // jmp rel +2
		{".beg: \n nop \n jmp.r .beg", "F000 90FF "}, // jmp rel -1
		{"jmp 1", "9201 "},
		{"jmp 0", "9200 "},
		{"jmp.ifz 0x10", "9610 "},
		{".beg: \n jmp.inz .beg", "9A00 "},
		{"jmp.inz .beg \n .beg: \n nop", "9A01 F000 "},
		{"nop \n jmp.igz.r .beg \n .beg: \n nop", "F000 9C01 F000 "},
		{"nop \n jmp.igz .beg \n .beg: \n nop", "F000 9E02 F000 "},
		{"loadi ra, 0x30", "A030 "},
		{"loadi rb, 0xFF", "A4FF "},
		{"loadi bp 32",    "A820 "},
		{"loadi.h sp 0xEE", "AEEE "},
		{"in rb", "B400 "},
		{"out ra", "B100 "},
		{"push bp", "BA00 "},
		{"pop sp", "BF00 "},
		{"nop \n call .fn \n nop \n .fn: \n nop", "F000 C003 F000 F000 "}, // call abs
		{".fn: \n nop \n call.r .fn \n nop", "F000 C1FF F000 "}, // call rel -1
		{"nop \n call.r .fn \n nop \n .fn: \n nop", "F000 C102 F000 F000 "}, // call rel + 2
		{"ret   ", "C200 "},
		{"ret", "C200 "},
		{"loada 0x45", "D045 "},
		{"loada 10", "D00A "},
		{"halt", "E000 "},
		{"nop", "F000 "},
		{"jmp 0xFF", "92FF "},
		{".static 1 buf \n .static 3 abc \n loadi ra, 10 \n store ra, .abc[2]","A00A 7203 "},
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
		"add ra", // missing argument
		"add 32", // missing argument (constant)
		"add 32, ra", // invalid first argument
		"add ra, ra, rb", // too many arguments
		"add ra, ra, rb, rc", // too many arguments
		"add ra, ra, 23", // too many arguments (constant)
		"add 32, rb", // invalid constant location
		"add ra, sp", // input 2 cannot be sp
		"move ra, rb", // invalid opcode
		"jmp.inz.bp 0x10", // bp flag is not valid for jmp
		"jmp.inz 0x10, 0x10", // only one constant allowed
		"call.rp 0x01", // rp is not valid for call
		"call .fn", // call a label that doesn't exists
		"out 0x10", // in/out/push/pop cannot have immediate
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
	// Should flash alternating 0x55 and 0xAA
	{"flasher_program.txt", "A055 A4FF B000 4000 9302 "},
	// Should output the fibonacci sequence
	{"fib_program.txt", "A001 A401 0800 8100 8600 B000 9302 "},
	// Tests using labels and relative jumps. Should just count up
	{"label.txt", "A00A 0101 B000 93FE "},
	// Note this program should produce the output 003C when run
	{"sum.txt", "A00A 7300 A00B 7301 A017 7302 A008 7303 A003 7304 A804 6200 0101 7200 1B01 8200 9FFB A804 A400 8100 6600 0000 1B01 8400 8200 9FFA B400 E000 " },
	// Should produce 0x0016 output
	{"var_test.txt", "A00A 7300 A00B A401 0000 7301 6300 6701 0000 7300 B000 E000 "},
	// Should produce 0x002B output
	{"array_test.txt", "A00A 7300 A00B 7301 A016 7302 A000 6702 0400 A801 6200 0400 1B01 8200 9FFC B400 E000 "},
	// Alternately flashes all the LEDs on and off for 400 "counts" each (~3 * 400 clocks)
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
