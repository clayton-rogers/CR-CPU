#include "compiler.h"

#include "c_to_asm.h"
#include "assembler.h"

#include <stdexcept>

Compiler_Return compile_tu(std::string filename, FileReader f) {
	Compiler_Return ret;
	
	std::string file_extension = get_file_extension(filename);

	if (file_extension == "c") {
		// TODO if any include paths are specified add them here.
		ret.assembly = c_to_asm(filename, f);
	} else if (file_extension == "s") {
		ret.assembly = f.read_file_from_directories(filename);
	} else {
		throw std::logic_error("Cannot handle file with extension: " + file_extension);
	}

	ret.item = assemble(ret.assembly);

	return ret;
}
