#include "assembler.h"
#include "simulator.h"
#include "file_io.h"
#include "machine_io.h"
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>
#include <iostream>
#include <iomanip>

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

struct TestProgram {

};

struct Test_Program {
	std::string name;
	std::string expected_assembly;
	bool expected_output_required;
	std::uint16_t expected_output;
};
// Used for both the assembler tests and benchmarks
static const std::vector<Test_Program> test_programs = {
	// Should flash alternating 0x55 and 0xAA
	{"flasher_program.txt", "A055 A4FF B100 4100 9202 ",
	false, 0x0000},
	// Should output the fibonacci sequence
	{"fib_program.txt", "A001 A401 B100 0100 8800 8100 8600 9202 ",
	false, 0x0000},
	// Tests using labels and relative jumps. Should just count up
	{"label.txt", "A00A 0301 B100 90FE ",
	false, 0x0000},
	// Note this program should produce the output 0x003C when run
	{"sum.txt", "D000 A00A 7200 A00B 7201 A017 7202 A008 7203 A003 7204 A804 6000 0301 7000 1B01 8200 9CFB A804 4500 8100 6400 0100 1B01 8400 8200 9CFA B500 E000 ",
	true, 0x003C},
	// Should produce 0x0016 output
	{"var_test.txt", "D000 A00A 7200 A00B A401 0100 7201 6200 6601 0100 7200 B100 E000 ",
	true, 0x0016},
	// Should produce 0x002B output
	{"array_test.txt", "D000 A00A 7200 A00B 7201 A016 7202 A000 6602 0400 A801 6000 0400 1B01 8200 9CFC B500 E000 ",
	true, 0x002B},
	// Alternately flashes all the LEDs on and off for 400 "counts" each (~3 * 400 clocks)
	{"flash.txt", "D000 A0FF B100 A090 A201 1301 9CFF A000 B100 A090 A201 1301 9CFF 9201 ",
	false, 0x0000},
	// Sums the numbers in an array using function calls, should output 0x0051
	{"call_test.txt", "AC00 AE01 A80B A00B 7000 A00C 7001 A00D 7002 A00E 7003 A00F 7004 A010 7005 D000 620B B200 620C 660D C10D B700 B200 620E B200 620F 6610 C106 B700 B700 0100 B100 E000 0100 6501 0100 C200 ",
	true, 0x0051},
};

TEST_CASE("Test assembler programs", "[asm]") {

	for (const auto& test_point : test_programs) {
		INFO(test_point.name);

		std::string program = read_file(std::string("./test_data/") + test_point.name);

		REQUIRE(program.length() != 0);

		std::string output = machine_inst_to_unformatted(assemble(program));

		CHECK(output == test_point.expected_assembly);

		auto instructions = assemble(program);
		Simulator sim(instructions, 256);
		int steps = 0;
		std::cout << std::hex << std::setfill('0');
		while (!sim.is_halted && steps < 200) {
			sim.step();
//			std::cout
//				<< std::setw(2) << steps << " "
//				<< std::setw(4) << sim.pc << " "
//				<< std::setw(4)  << sim.inst << " "
//				<< std::setw(4) << sim.next_inst << " "
//				<< std::setw(4) <<  sim.output
//				<< std::endl;
			++steps;
		}

		// Check that the simulated assembled program actually does as expected
		if (test_point.expected_output_required) {
			CHECK(sim.output == test_point.expected_output);
		}
	}
}

TEST_CASE("Benchmarks", "[bench]") {
	for (const auto& test_point : test_programs) {
		std::string program = read_file(std::string("./test_data/") + test_point.name);

		REQUIRE(program.length() != 0);

		BENCHMARK(std::string("Benchmark: ") + test_point.name) {
			return assemble(program);
		};
	}
}
