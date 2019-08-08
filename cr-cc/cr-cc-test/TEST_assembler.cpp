#include "assembler.h"
#include "file_reader.h"
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>

TEST_CASE("Always pass test case", "[asm]") {

	int a = 0;

	SECTION("Add one") {
		++a;

		CHECK(a == 1);
	}

	SECTION("Sub one") {
		--a;

		CHECK(a == -1);
	}
}

struct Test_Point {
	std::string input;
	std::string expected_out;
};

TEST_CASE("Test assembler", "[asm]") {
	std::vector<Test_Point> test_points = {
		{"ADD RA, RC, 10", "030A"}
	};

	for (const auto& test_point : test_points) {
		const std::string output = assemble(test_point.input);

		CHECK(output == test_point.expected_out+" ");
	}
}

TEST_CASE("Benchmarks", "[bench]") {
	std::string program = file_reader("./cr-cc-test/test_program.txt");

	BENCHMARK("Assemble test program") {	
		return assemble(program);
	};
}
