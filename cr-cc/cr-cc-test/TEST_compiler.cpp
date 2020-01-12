#include "compiler.h"
#include "file_io.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>
#include <iostream>

TEST_CASE("Test basic function of compiler", "[c]") {
	FileReader fr;
	fr.add_directory("./test_data");

	std::string test_filename = "basic.c";
	std::string result = compile(test_filename, fr);
	//std::cout << result << std::endl;

	test_filename = "first.c";
	result = compile(test_filename, fr);
	//std::cout << result << std::endl;

}
