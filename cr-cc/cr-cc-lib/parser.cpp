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
			{{TokenType::function_definition}, TokenType::function_definition_tail},
		}
	},
	{TokenType::function_definition_tail,
		{
			{{TokenType::function_definition}, {}},
		}
	},
	{TokenType::function_definition,
		{
			{{TokenType::type_specifier, TokenType::identifier, TokenType::open_parenth, TokenType::close_parenth, TokenType::compound_statement}, {}},
		}
	},
	{TokenType::type_specifier,
		{
			{{TokenType::key_int}, {}},// TODO add other types
		}
	},
	{TokenType::compound_statement,
		{
			{{TokenType::open_bracket, TokenType::close_bracket}, {}},
			{{TokenType::open_bracket, TokenType::statement_list, TokenType::close_bracket}, {}},
			// TODO add declaration list

		}
	},
	{TokenType::statement_list,
		{
			{{TokenType::statement}, TokenType::statement_tail},
		}
	},
	{TokenType::statement,
		{
			{{TokenType::jump_statement}, {}},
			// TODO add other types of statements
		}
	},
	{TokenType::statement_tail,
		{
			{{TokenType::statement}, {}},
		}
	},
	{TokenType::jump_statement,
		{
			{{TokenType::key_return, TokenType::semi_colon}, {}},
			{{TokenType::key_return, TokenType::expression, TokenType::semi_colon}, {}},
			// TODO add other types of jumps (break, continue, goto)
		}
	},
	{TokenType::expression,
		{
			{{TokenType::unary_expression}, {}},
			// TODO only for now, expressions will get more complicated
			{{TokenType::constant}, {}},
		}
	},
	{TokenType::unary_expression,
		{
			{{TokenType::sub, TokenType::expression}, {}},
			{{TokenType::tilda, TokenType::expression}, {}},
			{{TokenType::exclam, TokenType::expression}, {}},
		}
	},
};

static bool is_token_terminal(TokenType token) {
	// A token is terminal when it can no longer be broken into more tokens,
	// in this case the token will not appear in the grammar and will have been
	// recognized by the tokenizer.
	return C_GRAMMAR.count(token) != 1;
}

static int parse_node(ParseNode* node, const TokenList& token_list, int offset) {
	if (is_token_terminal(node->token.token_type)) {
		if (offset < static_cast<int>(token_list.size()) && node->token.token_type == token_list.at(offset).token_type) {
			node->token = token_list.at(offset);
			return 1; // Consume one token
		} else {
			return 0; // No match
		}
	} else {
		RuleList rule_list = C_GRAMMAR.at(node->token.token_type);

		for (const Rule& rule : rule_list) {
			int consumed_tokens = 0;

			// Try to match the required tokens if we get a match
			// then match any number of optionals
			for (const TokenType& required_token : rule.required) {
				ParseNode p;
				p.token.token_type = required_token;
				int ret = parse_node(&p, token_list, offset + consumed_tokens);
				if (ret == 0) {
					// failed to match this rule, restart from outer loop
					consumed_tokens = 0;
					node->children.clear();
					break;
				} else {
					node->children.push_back(p);
					consumed_tokens += ret;
				}
			}

			// If we did not match any required tokens, then try the next rule
			if (consumed_tokens == 0) {
				continue;
			}
			// else we will try to match as many optional parameters as possible before returning success
			if (rule.optional != TokenType::NONE) {
				while (true) {
					ParseNode p;
					p.token.token_type = rule.optional;
					int ret = parse_node(&p, token_list, offset + consumed_tokens);
					if (ret == 0) {
						// We did not match any (more) optionals so we're done
						break;
					} else {
						// We actually was this children
						for (const auto& child_node : p.children) {
							node->children.push_back(child_node);
						}
						consumed_tokens += ret;
					}
				}
			}

			// If we've gotten this far then we have matched the rule, return
			return consumed_tokens;
		}
	}

	return 0;
}

ParseNode parse(TokenList token_list) {
	//std::cout << "Following tokens are considered terminal:" << std::endl;
	//for (int i = 0; i < static_cast<int>(TokenType::last_token); ++i) {
	//	TokenType t = static_cast<TokenType>(i);
	//	if (is_token_terminal(t)) {
	//		std::cout << tokenType_to_string(t) << std::endl;
	//	}
	//}

	ParseNode root;
	root.token.token_type = TokenType::translation_unit;
	int tokens_parsed = parse_node(&root, token_list, 0);
	if (tokens_parsed == 0) {
		//throw std::logic_error("Failed to parse");
		std::cout << "FAILED TO PARSE" << std::endl;
	} else if (tokens_parsed != static_cast<int>(token_list.size())) {
		std::cout << "Parsed, but leftover tokens..." << std::endl;
	} else {
		std::cout << "Parsed " << tokens_parsed << " tokens!!" << std::endl;
	}

	return root;
}
