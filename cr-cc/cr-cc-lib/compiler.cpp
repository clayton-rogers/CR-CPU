#include "compiler.h"

#include "preprocessor.h"
#include "tokenizer.h"

#include <iostream>

class Compiler_State {
public:
	std::string code_original;
	std::string code_preprocessed;
	std::string code_stripped;

	TokenList token_list;
	std::string printed_tokens;
};


std::string compile(const std::string& tu_filename, FileReader fr) {
	Compiler_State cs;

	cs.code_original = fr.read_file_from_directories(tu_filename);
	cs.code_preprocessed = preprocess(cs.code_original, fr);
	cs.code_stripped = strip_comments(cs.code_preprocessed);
	cs.token_list = tokenize(cs.code_stripped);
	cs.printed_tokens = print_tokens(cs.token_list);

	return cs.printed_tokens;
}
