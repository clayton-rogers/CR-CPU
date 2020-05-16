#include <vector>
#include <string>

#include "file_io.h"

struct Compiler_Options {

	bool verbose = false;
	bool function_size = false;
	bool should_sim = false;
	bool compile_only = false;
	bool include_main = true;
	bool include_stdlib = true;
	bool should_exit = false;
	bool output_lib = false;
	bool output_map = false;
	bool output_hex = false;
	bool output_srec = false;
	bool output_srec_stdout = false;
	bool output_assembly = false;
	bool dump_object = false;
	int link_address = 0x200;
	std::vector<std::string> filenames;
	std::vector<std::string> include_paths;

	int sim_steps = 1000000;

	std::string get_out_filename(std::string extension) {
		if (output_filename_set) {
			return output_filename;
		} else {
			std::string base_name = "out";
			if (filenames.size() == 1) {
				base_name = get_base_filename(filenames.at(0));
			}
			return base_name + extension;
		}
	}

	void set_output_filename(std::string filename) {
		output_filename = filename;
		output_filename_set = true;
	}

private:
	std::string output_filename;
	bool output_filename_set = false;
};
