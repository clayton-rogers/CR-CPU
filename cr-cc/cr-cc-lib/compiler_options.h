#include <vector>
#include <string>


struct Compiler_Options {
	static const std::string DEFAULT_OUTPUT_FILENAME;

	bool verbose = false;
	bool function_size = false;
	bool should_sim = false;
	bool compile_only = false;
	bool include_main = true;
	bool include_stdlib = true;
	bool should_exit = false;
	bool output_lib = false;
	bool output_map = false;
	bool output_assembly = false;
	int link_address = 0x200;
	std::vector<std::string> filenames;
	std::vector<std::string> include_paths;
	std::string output_filename = DEFAULT_OUTPUT_FILENAME;
	int sim_steps = 1000000;
};
