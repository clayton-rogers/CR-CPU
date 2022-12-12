
#include "c_to_asm.h"
#include "file_io.h"
#include "compiler.h"
#include "utilities.h"
#include "simulator.h"
#include "linker.h"
#include "file_io.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>
#include <iostream>

TEST_CASE("Test basic function of compiler", "[c]")
{
	FileReader fr("./test_data");

	auto ret = compile_tu("./test_data/first.c", fr);
}

TEST_CASE("Compiler program Benchmarks", "[.][bench][c_bench]")
{
	FileReader fr("./stdlib/");
	fr.add_directory("./test_data/");

	struct Program {
		std::string name;
		std::vector<std::string> filenames;
	};

	std::vector<Program> programs = {
		{"first", {"./test_data/first.c"}},
		{"pong", {"./test_data/pong/asm.s", "./test_data/pong/main.c", "./test_data/pong/print_as_dec.c"} },
		{"mpmp_marching_band", {"./test_data/mpmp_marching_band/asm.s", "./test_data/mpmp_marching_band/mpmp_alt.c", "./test_data/mpmp_marching_band/mpmp_march.c"}}
	};

	auto stream = read_bin_file("./stdlib/stdlib.a");
	auto stdlib = Object::Object_Container::from_stream(stream);
	stream = read_bin_file("./stdlib/os.map");
	auto os_map = Object::Object_Container::from_stream(stream);

	for (const auto& program : programs) {
		BENCHMARK(std::string("Program name: ") + program.name)
		{

			std::vector<Object::Object_Container> objs;

			for (const auto& filename : program.filenames) {
				auto obj = compile_tu(filename, fr);
				objs.push_back(obj);
			}

			objs.push_back(stdlib);
			objs.push_back(os_map);

			return link(std::move(objs), 0x200);
		};
	}
}

static int get_expected_return(std::string filename)
{
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

TEST_CASE("Exaustive test of Compiler", "[c]")
{

	// These test programs were mostly stolen and slightly modified from:
	// https://github.com/nlsandler/write_a_c_compiler.git
	const std::string DIR = "test_data/valid_c/";
	auto dir_list = read_directory(DIR);

	for (const auto& item : dir_list) {
		INFO(item);
		FileReader fr("./stdlib");

		// Catching the exception in the test allows us to continue
		// rather than stopping at the first failed compile
		try {

			auto test_program = compile_tu(item, fr);
			auto init_main = compile_tu("./stdlib/main.s", fr); // this is what calls the main fn
			auto stream = read_bin_file("./stdlib/stdlib.a");
			auto stdlib = Object::Object_Container::from_stream(stream);

			std::vector<Object::Object_Container> objs;
			objs.push_back(init_main);
			objs.push_back(test_program);
			objs.push_back(stdlib); // link stdlib for those couple that need it
			auto exe = link(std::move(objs), 0x200);

			stream = read_bin_file("./stdlib/os.bin");
			auto os = Object::Object_Container::from_stream(stream);

			// Load the compiled code into the simulator and see that the return is correct
			Simulator sim;
			sim.load(os);
			sim.load_sim_overlay(); // sets up sp and jumps to 0x200, then halts
			sim.load(exe);

			sim.run_until_halted(55000); // longest test is currently isqrt.c
			CHECK(sim.get_state().is_halted == true);

			// Check that the program produced the desired result
			const int actual_program_output = sim.get_state().ra;
			const int expected_program_output = get_expected_return(item);

			CHECK(actual_program_output == expected_program_output);

		} catch (const std::exception& e) {
			std::string message("Unexpected exception with message:\n");
			message = message + e.what();
			INFO(message);
			CHECK(false);
			continue;
		}
	}
}

// Takes too long on autobuild
TEST_CASE("Compiler Benchmarks", "[.][long_bench][c_bench]")
{

	const std::string DIR = "test_data/valid_c/";
	auto dir_list = read_directory(DIR);

	FileReader fr("./stdlib");

	for (const auto& item : dir_list) {

		BENCHMARK(std::string("Filename: " + get_base_filename(item) + ".c"))
		{
			return compile_tu(item, fr);
		};
	}
}

TEST_CASE("Invalid C programs", "[c]")
{

	const std::string DIR = "test_data/invalid_c/";
	auto dir_list = read_directory(DIR);

	for (const auto& item : dir_list) {
		INFO(item);
		FileReader fr(".");
		fr.add_directory(DIR);

		CHECK_THROWS(compile_tu(item, fr));
	}
}

TEST_CASE("Whole C program", "[c]")
{
	FileReader fr("./stdlib");

	fr.add_directory("./test_data/whole_program/sub");

	std::vector<Object::Object_Container> objs;
	objs.push_back(compile_tu("./stdlib/main.s", fr));
	objs.push_back(compile_tu("./test_data/whole_program/main.c", fr));
	objs.push_back(compile_tu("./test_data/whole_program/add.c", fr));
	objs.push_back(compile_tu("./test_data/whole_program/sub/sub.c", fr));

	auto exe = link(std::move(objs), 0x200);




	Simulator sim;
	sim.load_sim_overlay();
	sim.load(exe);

	sim.run_until_halted(20000);
	CHECK(sim.get_state().is_halted == true);

	// Check that the program produced the desired result
	const int actual_program_output = sim.get_state().ra;
	const int expected_program_output = 77;

	CHECK(actual_program_output == expected_program_output);
}

TEST_CASE("Test file io filename methods", "[c]")
{
	std::string filename("test.txt");

	CHECK(get_base_filename(filename) == std::string("test"));
	CHECK(get_file_extension(filename) == std::string("txt"));
	CHECK(get_directory_name(filename) == std::string("./"));

	filename = "test/test.txt";

	CHECK(get_base_filename(filename) == std::string("test"));
	CHECK(get_file_extension(filename) == std::string("txt"));
	CHECK(get_directory_name(filename) == std::string("test/"));


	filename = "another_longer/filename.abc";

	CHECK(get_base_filename(filename) == std::string("filename"));
	CHECK(get_file_extension(filename) == std::string("abc"));
	CHECK(get_directory_name(filename) == std::string("another_longer/"));
}

TEST_CASE("Linker with multi segment files", "[link]")
{
	// These two relocation files are specially crafted so that addresses
	// that need relocation span a page boundary.
	//
	// Ex.
	// Originally the reference was to 0x0006
	// The relocation offset is 0x00FE
	//
	// If you handle the two bytes separately you get:
	// 00 + 00 =  00 high byte
	// 06 + FE = 104 low byte
	// Thus giving 0x0004 when it should be 0x0104
	// Issue was cause by the overflow of the low byte not incrementing the high byte

	using namespace Object;

	FileReader f("./stdlib");

	std::vector<Object_Container> objs;
	objs.push_back(compile_tu("./stdlib/main.s", f));
	objs.emplace_back(compile_tu("./test_data/relocation/relocation_asm1.s", f));
	objs.emplace_back(compile_tu("./test_data/relocation/relocation_asm2.s", f));

	auto exe = link(std::move(objs), 0x200);

	Simulator sim;
	sim.load_sim_overlay(); // sets up sp and jumps to 0x200, then halts
	sim.load(exe);

	sim.run_until_halted(20000);
	CHECK(sim.get_state().is_halted == true);
	CHECK(sim.get_state().ra == 0x0000);
}
