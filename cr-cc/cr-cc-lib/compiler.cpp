#include "compiler.h"

#include "file_io.h"

class Compiler_State {
public:
	std::string code_original;
	std::string code_preprocessed;
};


static std::string preprocess (const std::string& code) {
	return code;
}

std::string compile(const std::string& tu_filename) {
	Compiler_State cs;

	cs.code_original = read_file(tu_filename);
	cs.code_preprocessed = preprocess(cs.code_original);




	return cs.code_preprocessed;
}
