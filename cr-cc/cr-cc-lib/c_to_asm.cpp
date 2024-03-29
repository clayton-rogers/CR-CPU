#include "c_to_asm.h"

#include "preprocessor.h"
#include "tokenizer.h"
#include "parser.h"
#include "AST.h"

#include <iostream>

class Front_End_State {
public:
	std::string code_preprocessed;
	std::string code_stripped;

	TokenList token_list;
	std::string printed_tokens;
	ParseNode parse_tree;

};


std::string c_to_asm(const std::string& tu_filename, FileReader fr)
{
	Front_End_State cs;

	cs.code_preprocessed = preprocess(tu_filename, fr);
	cs.code_stripped = strip_comments(cs.code_preprocessed);
	cs.token_list = tokenize(cs.code_stripped);
	cs.printed_tokens = print_token_list(cs.token_list);
	cs.parse_tree = parse(cs.token_list);
	AST::AST ast(cs.parse_tree);

	return ast.generate_code();
}
