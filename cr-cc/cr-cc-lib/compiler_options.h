#include <vector>
#include <string>


struct Compiler_Options {
	static const std::string DEFAULT_OUTPUT_FILENAME;

	bool verbose = false;
	bool should_sim = false;
	bool compile_only = false;
	std::vector<std::string> filenames;
	std::string output_filename = DEFAULT_OUTPUT_FILENAME;
};