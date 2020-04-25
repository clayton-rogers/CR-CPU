#include "assembler.h"
#include "file_io.h"
#include "utilities.h"
#include "simulator.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <memory>

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
		{"and ra, rp", "2200 "},
		{"or ra, ra",  "3000 "},
		{"xor ra, ra", "4000 "},
		{"shftl rb, rb", "5500 "},
		{"shftl rb, 0x0F", "570F "},
		{"shftr rp, 1", "5A01 "},
		{"load sp, 10", "6E0A "},
		{"load ra, 0x10", "6210 "},
		{"load.rp rb, 10", "640A "},
		{"load.sp ra, -5", "61FB "},
		{"store sp, 10", "7E0A "},
		{"store ra, 0x10", "7210 "},
		{"store.rp rb, 10", "740A "},
		{"store.sp ra, -5", "71FB "},
		{"store sp, 10", "7E0A "},
		{"mov ra, rb", "8100 "},
		{"mov ra, sp", "8300 "},
		{"mov rp, sp", "8B00 "},
		{"jmp .end \n .end: \n nop", "9201 F000 "}, // jmp abs 1
		{"nop \n jmp.r .end \n nop \n .end: \n nop", "F000 9002 F000 F000 "}, // jmp rel +2
		{".beg: \n nop \n jmp.r .beg", "F000 90FF "}, // jmp rel -1
		{"jmp 1", "9201 "},
		{"jmp 0", "9200 "},
		{"jmp.z 0x10", "9610 "},
		{".beg: \n jmp.nz .beg", "9A00 "},
		{"jmp.nz .beg \n .beg: \n nop", "9A01 F000 "},
		{"nop \n jmp.gz.r .beg \n .beg: \n nop", "F000 9C01 F000 "},
		{"nop \n jmp.gz .beg \n .beg: \n nop", "F000 9E02 F000 "},
		{"loadi ra, 0x30", "A030 "},
		{"loadi rb, 0xFF", "A4FF "},
		{"loadi rp 32",    "A820 "},
		{"loadi.h sp 0xEE", "AEEE "},
		{".static 1 buf \n .static 1 a \n .static 2 b \n loadi ra, .a \n loadi.h ra, .a \n loadi rb,.b[1]", "A004 A200 A406 0000 0000 0000 0000 "},
		{".static 256 buf \n .static 1 a \n loadi ra, .a \n loadi.h ra,.a", "A002 A201 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 " },
		{"push rp", "B800 "},
		{"pop sp", "BD00 "},
		{"nop \n call .fn \n nop \n .fn: \n nop", "F000 C203 F000 F000 "}, // call abs
		{".fn: \n nop \n call.r .fn \n nop", "F000 C0FF F000 "}, // call rel -1
		{"nop \n call.r .fn \n nop \n .fn: \n nop", "F000 C002 F000 F000 "}, // call rel + 2
		{"ret   ", "C100 "},
		{"ret", "C100 "},
		{"loada 0x45", "D045 "},
		{"loada 10", "D00A "},
		{"loada .const \n .constant 0xFEAB const", "D0FE "},
		{"halt", "E000 "},
		{"nop", "F000 "},
		{"jmp 0xFF", "92FF "},
		{".static 1 buf \n .static 3 abc \n loadi ra, 10 \n store ra, .abc[2]","A00A 7205 0000 0000 0000 0000 "},
		{".static 1 var", "0000 "},
		{".static 1 var 0xff", "00FF "},
		{".static 1 var 1024", "0400 "},
		{".static 2 var 0xfafe 255", "FAFE 00FF "},
		{".static 1 bb 0xff \n .static 1 aa 0xaa", "00FF 00AA "}, // check that variables are stored in order declared
		{".static 0x0a bb", "0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 "}, // static variables size should accept hex
		{".static 6 text \"hello\"", "0068 0065 006C 006C 006F 0000 "}, // handling simple string
		{".static 6 text \"ab ab\"", "0061 0062 0020 0061 0062 0000 "}, // handling embeded space in string
		{".static 9 text \"ab ab ab\"", "0061 0062 0020 0061 0062 0020 0061 0062 0000 "}, // handling two spaces
		{".static 6 text \"HELLO\"", "0048 0045 004C 004C 004F 0000 "}, // handling upper case letters
		{"loadi ra, .const \n .constant 10 const", "A00A "},
		{"loadi ra, .const \n loadi.h ra, .const \n .constant 0xabcd const", "A0CD A2AB "},
		{"loada .const \n .constant 0xfedc const", "D0FE "},
		{".constant 123 .const", ""}, // by itself constants produce no code
		{".constant 0x00ff const \n loadi ra, .const[1] \n loadi.h ra, .const[1]", "A000 A201 "}, // array offsets should roll over into high byte
		{".label: \n loadi ra, .label \n loadi.h ra .label ", "A000 A200 "},
		{".label: \n .export label \n loadi ra, .label \n loadi.h ra .label ", "A000 A200 "},
		{".extern label \n loadi ra, .label \n loadi.h ra .label ", "A000 A200 "},
		{".extern label \n loadi ra, .label \n loadi.h ra .label \n .extern label", "A000 A200 "}, // duplicate labels are allowed
	};

	for (const auto& test_point : test_points) {
		INFO(test_point.input);

		auto object_file = assemble(test_point.input);
		std::string output = machine_inst_to_simple_hex(std::get<Object::Object_Type>(object_file.contents).machine_code);

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
		"add ra, -1", // immediate for arithmetic cannot be negative (because it is not sign extended
		"move ra, rb", // invalid opcode
		"load.rp.sp ra, 0x10", // two opposite flags are not allowed
		"jmp.nz.rp 0x10", // rp flag is not valid for jmp
		"jmp.nz 0x10, 0x10", // only one constant allowed
		"jmp.nz.z 0x10", // two oposite flags are not allowed
		"jmp.nz.z.r 0x10", // two oposite flags are not allowed
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
		".static 2 my_var 12", // must specify none or all parameters
		".static 1 my_var -32769", // constant out of range
		".static 1 var 65536", // constant out of range
		".static 2 text \"a", // missing closing quote
		".static 4 text \"ad\"", // incorrect length, should be 3
		"in ra", // In and out are no longer valid instructions
		"out ra",
		"in",
		"out",
		".constant name 231", // order of name and number are reversed
		".constant 65536 name", // constant is too large
		"shftl ra, 0x10", // constant must be 0 .. 15
		"shftr rb, -1", // constant must be 0 .. 15
		".text_offset 0x1020 013", // text offset only takes one argument
		".text_offset ", // text offset must take an argument
		".extern ", // missing label
		".export ", // missing label
		".export label \n .label: \n nop", // trying to export label that doesn't exist (yet)
	};

	for (const auto& test_point : test_points) {
		INFO(test_point);
		CHECK_THROWS(assemble(test_point));
	}
}

struct Test_Program {
	std::string name;
	bool expected_output_required;
	std::uint16_t expected_output;
};
// Used for both the assembler tests and benchmarks
static const std::vector<Test_Program> test_programs = {
	// Test of using the stack for call argument. Is sensitive to wether SP is top or top+1 of the stack
	{"call_test_2.s", true, 0x82},
	// Simple test of push and pop
	{"push_pop.s", true, 0xC3},
	// Tries to implement usleep using bogomips
	{"bogomips.s", true, 105}, // 50 iterations are performed, 2 clocks per iteration, 2 clocks for ret, 2 clocks to read timer, 1 clock for call
	// Note this program should produce the output 0x003C when run
	{"sum.s", true, 0x003C},
	// Should produce 0x0016 output
	{"var_test.s", true, 0x0016},
	// Should produce 0x002B output
	{"array_test.s", true, 0x002B},
	// Alternately flashes all the LEDs on and off for 400 "counts" each (~3 * 400 clocks)
	{"flash.s", false, 0x0000},
	// Sums the numbers in an array using function calls, should output 0x0051
	{"call_test.s", true, 0x0051},
	// Sums a few numbers stored in the data segment
	{"static_data.s", true, 55},
	// Should flash alternating 0x55 and 0xAA
	{"flasher_program.s", false, 0x0000 },
	// Should output the fibonacci sequence
	{"fib_program.s", false, 0x0000},
	// Tests using labels and relative jumps. Should just count up
	{"label.s", false, 0x0000},
	// Test that compiling the os works and it runs, but don't test output.
	{"os.s", false, 0x0000},
	// Test that compiling hello world at offset 0x100 work, but don't test output
	{"hello_world.s", false, 0x0000},
};

TEST_CASE("Test assembler programs", "[asm]") {

	for (const auto& test_point : test_programs) {
		INFO(test_point.name);

		std::string program = read_file(std::string("./test_data/") + test_point.name);

		REQUIRE(program.length() != 0);

		auto object_file = assemble(program);

		Simulator sim;
		sim.load(0, std::get<Object::Object_Type>(object_file.contents).machine_code);
		sim.run_until_halted(200);

		// Check that the simulated assembled program actually does as expected
		if (test_point.expected_output_required) {
			CHECK(sim.get_state().output == test_point.expected_output);
		}
	}
}

TEST_CASE("Assembler Benchmarks", "[.][bench]") {
	for (const auto& test_point : test_programs) {
		std::string program = read_file(std::string("./test_data/") + test_point.name);

		REQUIRE(program.length() != 0);

		BENCHMARK(std::string("Benchmark: ") + test_point.name) {
			return assemble(program);
		};
	}
}

static bool operator==(const Object::Relocation& a, const Object::Relocation& b) {
	if (a.type != b.type) {
		return false;
	} else if (a.offset != b.offset) {
		return false;
	} else {
		return true;
	}
}

static bool operator==(const Object::External_Reference& a, const Object::External_Reference& b) {
	if (a.name != b.name) {
		return false;
	} else if (a.type != b.type) {
		return false;
	} else if (a.locations != b.locations) {
		return false;
	} else {
		return true;
	}
}

static bool operator==(const Object::Exported_Symbol& a, const Object::Exported_Symbol& b) {
	if (a.name != b.name) {
		return false;
	} else if (a.type != b.type) {
		return false;
	} else if (a.offset != b.offset) {
		return false;
	} else {
		return true;
	}
}

TEST_CASE("Assembler object Relocations", "[asm]") {
	using namespace Object;

	std::string input = read_file("./test_data/relocation_test.s");
	auto object_file = assemble(input);
	auto object = std::get<Object_Type>(object_file.contents);

	// Check that we have the required relocations
	{
		const auto& reloc = object.relocations;

		CHECK(reloc.at(0) == Relocation{ HI_LO_TYPE::HI_BYTE, 0x0B });
		CHECK(reloc.at(1) == Relocation{ HI_LO_TYPE::LO_BYTE, 0x0C });
		CHECK(reloc.at(2) == Relocation{ HI_LO_TYPE::LO_BYTE, 0x0D });
		CHECK(reloc.at(3) == Relocation{ HI_LO_TYPE::LO_BYTE, 0x0E });
		CHECK(reloc.at(4) == Relocation{ HI_LO_TYPE::LO_BYTE, 0x0F });
		CHECK(reloc.at(5) == Relocation{ HI_LO_TYPE::HI_BYTE, 0x10 });
		CHECK(reloc.at(6) == Relocation{ HI_LO_TYPE::LO_BYTE, 0x11 });
	}
	// Check the output machine code
	{
		std::string expected_machine_code =
			"F000 F000 F000 F000 D081 C223 9223 7223 6223 A281 A023 D000 C204 9204 7204 6204 A200 A004 ";
		std::string actual_machine_code = machine_inst_to_simple_hex(object.machine_code);

		CHECK(expected_machine_code == actual_machine_code);
	}
}

TEST_CASE("Assembler object External references", "[asm]") {
	using namespace Object;

	std::string input = read_file("./test_data/external_references_test.s");
	auto object_file = assemble(input);
	auto object = std::get<Object_Type>(object_file.contents);

	// Check that we have the required external refs
	{
		const auto& refs = object.external_references;

		CHECK(refs.at(0) == External_Reference{ "bar", HI_LO_TYPE::HI_BYTE, {} });
		CHECK(refs.at(1) == External_Reference{ "bar", HI_LO_TYPE::LO_BYTE, {} });
		CHECK(refs.at(2) == External_Reference{ "foo", HI_LO_TYPE::HI_BYTE, {0x04} });
		CHECK(refs.at(3) == External_Reference{ "foo", HI_LO_TYPE::LO_BYTE, {0x05} });
	}
	// Check the output machine code
	{
		std::string expected_machine_code =
			"F000 F000 F000 F000 D000 C200 ";
		std::string actual_machine_code = machine_inst_to_simple_hex(object.machine_code);

		CHECK(expected_machine_code == actual_machine_code);
	}
}

TEST_CASE("Assembler object Exported referencs", "[asm]") {
	using namespace Object;

	std::string input = read_file("./test_data/exported_references_test.s");
	auto object_file = assemble(input);
	auto object = std::get<Object_Type>(object_file.contents);

	// Check that we have the required external refs
	{
		const auto& exported = object.exported_symbols;

		CHECK(exported.at(0) == Exported_Symbol{ "add_numbers", Symbol_Type::FUNCTION, 0x02 });
		CHECK(exported.at(1) == Exported_Symbol{ "main", Symbol_Type::FUNCTION, 0x04 });
	}
	// Check that we have the required relocations
	{
		const auto& reloc = object.relocations;

		CHECK(reloc.at(0) == Relocation{ HI_LO_TYPE::HI_BYTE, 0x06 });
		CHECK(reloc.at(1) == Relocation{ HI_LO_TYPE::LO_BYTE, 0x07 });
	}
	// Check the output machine code
	{
		std::string expected_machine_code =
			"F000 F000 0100 C100 A00A A40B D000 C202 C100 ";
		std::string actual_machine_code = machine_inst_to_simple_hex(object.machine_code);

		CHECK(expected_machine_code == actual_machine_code);
	}
}
