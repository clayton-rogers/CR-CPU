#pragma once

#include "file_io.h"

#include <string>
#include <vector>
#include <cstdint>

struct Compiler_Return {
	std::string assembly;
	std::vector<std::uint16_t> machine_code;
	std::uint16_t load_address = 0;
};

Compiler_Return compile_tu(std::string filename, FileReader f);
