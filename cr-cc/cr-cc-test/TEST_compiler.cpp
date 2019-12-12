
#include "compiler.h"



#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

#include <string>
#include <iostream>

TEST_CASE("Test basic function of compiler", "[c]") {
	std::string test_filename = "./test_data/basic.c";

	std::string result = compile(test_filename);

	std::cout << result << std::endl;

}