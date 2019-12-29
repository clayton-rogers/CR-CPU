#include "compiler.h"

#include "preprocessor.h"

#include <iostream>

class Compiler_State {
public:
	std::string code_original;
	std::string code_preprocessed;
	std::string code_stripped;
};





std::string compile(const std::string& tu_filename, FileReader fr) {
	Compiler_State cs;

	cs.code_original = fr.read_file_from_directories(tu_filename);
	cs.code_preprocessed = preprocess(cs.code_original, fr);
	cs.code_stripped = strip_comments(cs.code_preprocessed);

	return cs.code_stripped;
}
