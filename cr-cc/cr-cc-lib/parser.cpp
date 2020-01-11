#include "parser.h"

#include <map>
#include <iostream> // TODO remove


using TokenTypeList = std::vector<TokenType>;

struct Rule {
	TokenTypeList required;
	TokenType optional;
};

using RuleList = std::vector<Rule>;

const static std::map<TokenType, RuleList> C_GRAMMAR = {
	{TokenType::translation_unit,
		{
			{{TokenType::function_definition}, {}},
		}
	},
	{TokenType::function_definition,
		{
			{{TokenType::type_specifier, TokenType::identifier, TokenType::open_parenth, TokenType::close_bracket, TokenType::compound_statement}, {}},
		}
	},
};


ParseNode parse(TokenList token_list) {
	ParseNode root;



	return root;
}
