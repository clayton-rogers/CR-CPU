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
			{{TokenType::open_bracket, TokenType::block_item_list, TokenType::close_bracket}, {}},
			{{TokenType::open_bracket, TokenType::close_bracket}, {}},
		}
	},
	{TokenType::block_item_list,
		{
			{{}, TokenType::block_item},
		}
	},
	{TokenType::block_item,
		{
			{{TokenType::declaration}, },
			{{TokenType::statement}, },
		}
	},
	{TokenType::declaration,
		{
			//{{TokenType::type_specifier, TokenType::semi_colon}, {}},// TODO for struct declaration
			{{TokenType::type_specifier, TokenType::init_declarator_list, TokenType::semi_colon}, {}},
		}
	},
	{TokenType::init_declarator_list,
		{
			{{TokenType::init_declarator}, TokenType::init_declarator_list_tail},
		}
	},
	{TokenType::init_declarator_list_tail,
		{
			{{TokenType::comma, TokenType::init_declarator}, {}},
		}
	},
	{TokenType::init_declarator,
		{
			{{TokenType::identifier, TokenType::equals, TokenType::expression}, {}}, // initialized
			{{TokenType::identifier}, {}}, // non initialized
		}
	},
	{TokenType::statement,
		{
			{{TokenType::compound_statement}, {}},
			{{TokenType::jump_statement}, {}},
			{{TokenType::if_statement}, {}},
			{{TokenType::for_statement}, {}},
			{{TokenType::while_statement}, {}},
			{{TokenType::do_while_statement}, {}},
			{{TokenType::break_statement}, {}},
			{{TokenType::continue_statement}, {}},
			{{TokenType::expression_statement}, {}},
			// TODO add other types of statements
		}
	},
	{TokenType::jump_statement,
		{
			// TODO for now don't accept return without an expression
			//{{TokenType::key_return, TokenType::semi_colon}, {}},
			{{TokenType::key_return, TokenType::expression, TokenType::semi_colon}, {}},
			// TODO add other types of jumps (break, continue, goto)
		}
	},
	{TokenType::if_statement,
		{
			// If with else
			{{TokenType::key_if, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth,
				TokenType::statement, TokenType::key_else, TokenType::statement}, {}},
			// If without else
			{{TokenType::key_if, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth,
				TokenType::statement}, {}},
		}
	},
	{TokenType::while_statement,
		{
			{{TokenType::key_while, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::statement}, {}},
		}
	},
	{TokenType::do_while_statement,
		{
			{{TokenType::key_do, TokenType::statement, TokenType::key_while,
			TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::semi_colon}, {}},
		}
	},
	{TokenType::for_statement,
		{
			{{TokenType::key_for, TokenType::open_parenth, TokenType::for_init_expression,
			TokenType::for_condition_expression, TokenType::for_increment_expression, TokenType::statement}, {}},
		}
	},
	{TokenType::for_init_expression,
		{
			{{TokenType::declaration}, {}},
			{{TokenType::expression, TokenType::semi_colon}, {}},
			{{TokenType::semi_colon}, {}},
		}
	},
	{TokenType::for_condition_expression,
		{
			// Condition expression is optional, if omitted it is assumed 1
			{{TokenType::expression, TokenType::semi_colon}, {}},
			{{TokenType::semi_colon}, {}},
		}
	},
	{TokenType::for_increment_expression,
		{
			// Incremente expression is optional. We must include the close parenth
			// because every non-terminal token must consume at least one token to be valid
			{{TokenType::expression, TokenType::close_parenth}, {}},
			{{TokenType::close_parenth}, {}},
		}
	},
	{TokenType::break_statement,
		{
			{{TokenType::key_break, TokenType::semi_colon}, {}},
		}
	},
	{TokenType::continue_statement,
		{
			{{TokenType::key_continue, TokenType::semi_colon}, {}},
		}
	},
	{TokenType::expression_statement,
		{
			{{TokenType::expression, TokenType::semi_colon}, {}},
			{{TokenType::semi_colon}, {}},
		}
	},
	{TokenType::expression,
		{
			{{TokenType::identifier, TokenType::equals, TokenType::expression}, {}},
			{{TokenType::conditional_exp}, {}},
		}
	},
	{TokenType::conditional_exp,
		{
			{{TokenType::logical_or_exp}, TokenType::logical_or_exp_tail},
		}
	},
	{TokenType::logical_or_exp_tail,
		{
			{{TokenType::question, TokenType::expression, TokenType::colon, TokenType::conditional_exp}, {}},
		}
	},
	{TokenType::logical_or_exp,
		{
			{{TokenType::logical_and_exp}, TokenType::logical_and_exp_tail},
		}
	},
	{TokenType::logical_and_exp_tail,
		{
			{{TokenType::or_op, TokenType::logical_and_exp}, {}},
		}
	},
	{TokenType::logical_and_exp,
		{
			{{TokenType::equality_exp}, TokenType::equality_exp_tail},
		}
	},
	{TokenType::equality_exp_tail,
		{
			{{TokenType::and_op, TokenType::equality_exp}, {}},
		}
	},
	{TokenType::equality_exp,
		{
			{{TokenType::relational_exp}, TokenType::relational_exp_tail},
		}
	},
	{TokenType::relational_exp_tail,
		{
			{{TokenType::ne_op, TokenType::relational_exp}, {}},
			{{TokenType::eq_op, TokenType::relational_exp}, {}},
		}
	},
	{TokenType::relational_exp,
		{
			{{TokenType::additive_exp}, TokenType::additive_exp_tail},
		}
	},
	{TokenType::additive_exp_tail,
		{
			{{TokenType::less_than, TokenType::additive_exp}, {}},
			{{TokenType::greater_than, TokenType::additive_exp}, {}},
			{{TokenType::le_op, TokenType::additive_exp}, {}},
			{{TokenType::ge_op, TokenType::additive_exp}, {}},
		}
	},
	{TokenType::additive_exp,
		{
			{{TokenType::term}, TokenType::term_tail},
		}
	},
	{TokenType::term_tail,
		{
			{{TokenType::add, TokenType::term}, {}},
			{{TokenType::sub, TokenType::term}, {}},
		}
	},
	{TokenType::term,
		{
			{{TokenType::factor}, TokenType::factor_tail},
		}
	},
	{TokenType::factor_tail,
		{
			{{TokenType::star, TokenType::factor}, {}},
			{{TokenType::div, TokenType::factor}, {}},
		}
	},
	{TokenType::factor,
		{
			{{TokenType::open_parenth, TokenType::expression, TokenType::close_parenth}, {}},
			{{TokenType::identifier}, {}},
			{{TokenType::unary_expression}, {}},
			{{TokenType::constant}, {}},
		}
	},
	{TokenType::unary_expression,
		{
			{{TokenType::sub, TokenType::factor}, {}},
			{{TokenType::tilda, TokenType::factor}, {}},
			{{TokenType::exclam, TokenType::factor}, {}},
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
			if (consumed_tokens == 0 && !rule.required.size() == 0) {
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
		throw std::logic_error("Failed to parse");
		//std::cout << "FAILED TO PARSE" << std::endl;
	} else if (tokens_parsed != static_cast<int>(token_list.size())) {
		throw std::logic_error("Parsed with tokens left over");
		//std::cout << "Parsed, but leftover tokens..." << std::endl;
	} else {
		//std::cout << "Parsed " << tokens_parsed << " tokens!!" << std::endl;
	}

	return root;
}
