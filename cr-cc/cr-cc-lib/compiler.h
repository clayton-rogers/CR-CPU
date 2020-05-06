#pragma once

#include "file_io.h"
#include "object_code.h"

#include <string>
#include <vector>
#include <cstdint>

struct Compiler_Return {
	std::string assembly;
	Object::Object_Container item;
};

Compiler_Return compile_tu(std::string filename, FileReader f);
