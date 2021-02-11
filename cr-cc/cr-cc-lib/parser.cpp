#include "parser.h"

#include <map>
#include <iostream> // TODO remove


using TokenTypeList = std::vector<TokenType>;

struct Rule {
	TokenTypeList required;
	TokenType optional;
};

using RuleList = std::vector<Rule>;

static int new_parse_node(ParseNode* node, const TokenList& token_list, int offset) {
	struct TokenPack {
		enum RuleType {
			DIRECT,
			ZERO_OR_ONE,
			ZERO_OR_MORE,
			ONE_OR_MORE,
		} rule_type;
		//TokenType token;
		std::vector<TokenType> tokens;

		explicit TokenPack(RuleType rule_type, std::vector<TokenType> tokens) : rule_type(rule_type), tokens(tokens) {}
		TokenPack(TokenType token) : rule_type(DIRECT), tokens{ token } {}
	};
	using NewRule = std::vector<TokenPack>;

	auto optional = [](TokenType token) {
		return TokenPack(TokenPack::ZERO_OR_ONE, { token });
	};
	auto optional_pack = [](std::vector<TokenType> tokens) {
		return TokenPack(TokenPack::ZERO_OR_ONE, tokens);
	};

	auto one_or_more = [](TokenType token) {
		return TokenPack(TokenPack::ONE_OR_MORE, { token });
	};
	auto one_or_more_pack = [](std::vector<TokenType> tokens) {
		return TokenPack(TokenPack::ONE_OR_MORE, tokens);
	};

	auto zero_or_more = [](TokenType token) {
		return TokenPack(TokenPack::ZERO_OR_MORE, { token });
	};
	auto zero_or_more_pack = [](std::vector<TokenType> tokens) {
		return TokenPack(TokenPack::ZERO_OR_MORE, tokens);
	};


	using NewRuleList = std::vector<NewRule>;
	std::map<TokenType, NewRuleList> NEW_C_GRAMMAR;


	NEW_C_GRAMMAR[TokenType::translation_unit] = {
		{one_or_more(TokenType::external_declaration)}
	};
	// TODO delete external_declaration_tail
	NEW_C_GRAMMAR[TokenType::external_declaration] = {
		{ TokenType::function },
		{ TokenType::declaration },
	};
	NEW_C_GRAMMAR[TokenType::function] = {
		{ TokenType::function_declaration },
		{ TokenType::function_definition },
	};
	NEW_C_GRAMMAR[TokenType::function_declaration] = {
		{TokenType::declaration_specifier, TokenType::identifier, TokenType::open_parenth, optional(TokenType::parameter_list), TokenType::close_parenth, TokenType::semi_colon},
	};
	NEW_C_GRAMMAR[TokenType::function_definition] = {
		{TokenType::declaration_specifier, TokenType::identifier, TokenType::open_parenth, optional(TokenType::parameter_list), TokenType::close_parenth, TokenType::compound_statement},
	};
	NEW_C_GRAMMAR[TokenType::parameter_list] = {
		{TokenType::parameter_declaration, zero_or_more_pack({TokenType::comma, TokenType::parameter_declaration})},
	};
	NEW_C_GRAMMAR[TokenType::parameter_declaration] = {
		{TokenType::declaration_specifier, optional(TokenType::init_declarator)},
	};
	NEW_C_GRAMMAR[TokenType::declaration_specifier] = {
		{TokenType::key_int}, // TODO add other types and specifiers
	};
	NEW_C_GRAMMAR[TokenType::compound_statement] = {
		{TokenType::open_bracket, optional(TokenType::block_item_list), TokenType::close_bracket},
	};
	// TODO block item list could be eliminated?
	NEW_C_GRAMMAR[TokenType::block_item_list] = {
		{zero_or_more(TokenType::block_item)},
	};
	NEW_C_GRAMMAR[TokenType::block_item] = {
		{TokenType::declaration},
		{TokenType::statement},
	};
	NEW_C_GRAMMAR[TokenType::declaration] = {
		//{TokenType::type_specifer, TokenType::semi_colon}, // TODO for struct declaration
		{TokenType::declaration_specifier, TokenType::init_declarator_list, TokenType::semi_colon},
	};
	NEW_C_GRAMMAR[TokenType::init_declarator_list] = {
		{TokenType::init_declarator, zero_or_more_pack({TokenType::comma, TokenType::init_declarator})},
	};
	NEW_C_GRAMMAR[TokenType::init_declarator] = {
		// TODO could make optional
		{TokenType::declarator, TokenType::equals, TokenType::expression},
		{TokenType::declarator},
	};
	NEW_C_GRAMMAR[TokenType::declarator] = {
		// TODO optimize
		//{optional(TokenType::pointer), TokenType::direct_declarator},
		{TokenType::pointer, TokenType::direct_declarator},
		{TokenType::direct_declarator},
	};
	NEW_C_GRAMMAR[TokenType::direct_declarator] = {
		{TokenType::identifier, zero_or_more_pack({TokenType::open_square_bracket, TokenType::constant, TokenType::close_square_bracket})}, // var/pointer
								// ^ technically should be const expression
	};
	NEW_C_GRAMMAR[TokenType::pointer] = {
		{one_or_more(TokenType::star)},
	};
	NEW_C_GRAMMAR[TokenType::statement] = {
		{TokenType::compound_statement},
		{TokenType::jump_statement},
		{TokenType::if_statement},
		{TokenType::for_statement},
		{TokenType::while_statement},
		{TokenType::do_while_statement},
		{TokenType::break_statement},
		{TokenType::continue_statement},
		{TokenType::expression_statement},
	};
	NEW_C_GRAMMAR[TokenType::jump_statement] = {
		// TODO for now don't accept return without an expression
		//{TokenType::key_return, TokenType::semi_colon},
		{TokenType::key_return, TokenType::expression, TokenType::semi_colon},
		// TODO goto?
	};
	NEW_C_GRAMMAR[TokenType::if_statement] = {
		// If with else
		{TokenType::key_if, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::statement, TokenType::key_else, TokenType::statement},
		// If without else
		{TokenType::key_if, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::statement},
	};
	NEW_C_GRAMMAR[TokenType::while_statement] = {
		{TokenType::key_while, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::statement},
	};
	NEW_C_GRAMMAR[TokenType::do_while_statement] = {
		{TokenType::key_do, TokenType::statement, TokenType::key_while, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::semi_colon},
	};
	NEW_C_GRAMMAR[TokenType::for_statement] = {
		{TokenType::key_for, TokenType::open_parenth, TokenType::for_init_expression, TokenType::for_condition_expression, TokenType::for_increment_expression, TokenType::statement},
	};
	NEW_C_GRAMMAR[TokenType::for_init_expression] = {
		{TokenType::declaration},
		{TokenType::expression, TokenType::semi_colon},
		{TokenType::semi_colon},
	};
	NEW_C_GRAMMAR[TokenType::for_condition_expression] = {
		// TODO optimize to one line
		//{optional(TokenType::expression), TokenType::semi_colon},

		{TokenType::expression, TokenType::semi_colon},
		{TokenType::semi_colon},
	};
	NEW_C_GRAMMAR[TokenType::for_increment_expression] = {
		// TODO optimize to one line
		//{optional(TokenType::expression), TokenType::close_parenth},

		{TokenType::expression, TokenType::close_parenth},
		{TokenType::close_parenth},
	};
	NEW_C_GRAMMAR[TokenType::break_statement] = {
		{TokenType::key_break, TokenType::semi_colon},
	};
	NEW_C_GRAMMAR[TokenType::continue_statement] = {
		{TokenType::key_continue, TokenType::semi_colon},
	};
	NEW_C_GRAMMAR[TokenType::expression_statement] = {
		// TODO optimize to one line
		//{optional(TokenType::expression), TokenType::semi_colon},
		{TokenType::expression, TokenType::semi_colon},
		{TokenType::semi_colon},
	};
	NEW_C_GRAMMAR[TokenType::expression] = {
		{TokenType::unary_expression, TokenType::equals, TokenType::expression},
		{TokenType::conditional_exp},
	};
	NEW_C_GRAMMAR[TokenType::conditional_exp] = {
		{TokenType::logical_or_exp, zero_or_more_pack({TokenType::question, TokenType::expression, TokenType::colon, TokenType::conditional_exp})},
	};
	NEW_C_GRAMMAR[TokenType::logical_or_exp] = {
		{TokenType::logical_and_exp, zero_or_more_pack({TokenType::or_op, TokenType::logical_and_exp})},
	};
	NEW_C_GRAMMAR[TokenType::logical_and_exp] = {
		{TokenType::equality_exp, zero_or_more_pack({TokenType::and_op, TokenType::equality_exp})},
	};
	NEW_C_GRAMMAR[TokenType::equality_exp] = {
		{TokenType::relational_exp, zero_or_more(TokenType::relational_exp_tail)},
	};
	NEW_C_GRAMMAR[TokenType::relational_exp_tail] = {
		{TokenType::ne_op, TokenType::relational_exp},
		{TokenType::eq_op, TokenType::relational_exp},
	};
	NEW_C_GRAMMAR[TokenType::relational_exp] = {
		{TokenType::shift_exp, zero_or_more(TokenType::shift_exp_tail)},
	};
	NEW_C_GRAMMAR[TokenType::shift_exp_tail] = {
		{TokenType::less_than, TokenType::shift_exp},
		{TokenType::greater_than, TokenType::shift_exp},
		{TokenType::le_op, TokenType::shift_exp},
		{TokenType::ge_op, TokenType::shift_exp},
	};
	NEW_C_GRAMMAR[TokenType::shift_exp] = {
		{TokenType::additive_exp, zero_or_more(TokenType::additive_exp_tail)},
	};
	NEW_C_GRAMMAR[TokenType::additive_exp_tail] = {
		{TokenType::left_op, TokenType::additive_exp},
		{TokenType::right_op, TokenType::additive_exp},
	};
	NEW_C_GRAMMAR[TokenType::additive_exp] = {
		{TokenType::term, zero_or_more(TokenType::term_tail)},
	};
	NEW_C_GRAMMAR[TokenType::term_tail] = {
		{TokenType::add, TokenType::term},
		{TokenType::sub, TokenType::term},
	};
	NEW_C_GRAMMAR[TokenType::term] = {
		{TokenType::factor, zero_or_more(TokenType::factor_tail)},
	};
	NEW_C_GRAMMAR[TokenType::factor_tail] = {
		{TokenType::star, TokenType::factor}, // mult
		{TokenType::div, TokenType::factor}, // div
		{TokenType::percent, TokenType::factor}, // modulo
	};
	NEW_C_GRAMMAR[TokenType::factor] = {
		{TokenType::open_parenth, TokenType::expression, TokenType::close_parenth},
		{TokenType::function_call},
		{TokenType::unary_expression},
		{TokenType::constant},
	};
	NEW_C_GRAMMAR[TokenType::function_call] = {
		{TokenType::identifier, TokenType::open_parenth, TokenType::close_parenth},
		{TokenType::identifier, TokenType::open_parenth, TokenType::argument_expression_list, TokenType::close_parenth},
	};
	NEW_C_GRAMMAR[TokenType::argument_expression_list] = {
		{TokenType::expression, zero_or_more(TokenType::argument_expression_list_tail)},
	};
	NEW_C_GRAMMAR[TokenType::argument_expression_list_tail] = {
		{TokenType::comma, TokenType::expression},
	};
	NEW_C_GRAMMAR[TokenType::unary_expression] = {
		{TokenType::sub, TokenType::factor},
		{TokenType::tilda, TokenType::factor},
		{TokenType::exclam, TokenType::factor},
		{TokenType::ampersand, TokenType::identifier},
		{TokenType::star, TokenType::identifier},
		{TokenType::identifier, TokenType::open_square_bracket, TokenType::expression, TokenType::close_square_bracket},
		{TokenType::identifier},
	};

	auto local_is_token_terminal = [&NEW_C_GRAMMAR](TokenType token) {
		return NEW_C_GRAMMAR.count(token) != 1;
	};

	if (local_is_token_terminal(node->token.token_type)) {
		if (offset < static_cast<int>(token_list.size()) && node->token.token_type == token_list.at(offset).token_type) {
			node->token = token_list.at(offset);
			return 1; // Consume one token
		} else {
			return 0; // No match
		}
	} else {
		NewRuleList rule_list = NEW_C_GRAMMAR.at(node->token.token_type);

		for (const NewRule& rule : rule_list) {
			int consumed_tokens = 0;

			for (const TokenPack& token_pack : rule) {

				// If this token_pack is required, force us to get all of them
				if (token_pack.rule_type == TokenPack::DIRECT || token_pack.rule_type == TokenPack::ONE_OR_MORE) {
					for (const TokenType& token : token_pack.tokens) {
						ParseNode p;
						p.token.token_type = token;
						int ret = new_parse_node(&p, token_list, offset + consumed_tokens);
						if (ret == 0) {
							consumed_tokens = 0;
							node->children.clear();
							break;
						} else {
							node->children.push_back(p);
							consumed_tokens += ret;
						}
					}

					if (consumed_tokens == 0) {
						goto next_rule;
					}
				}

				// If this token may optionally have more than one of itself then try to get more
				if (token_pack.rule_type == TokenPack::ONE_OR_MORE ||
					token_pack.rule_type == TokenPack::ZERO_OR_MORE ||
					token_pack.rule_type == TokenPack::ZERO_OR_ONE) {
					while (true) {
						std::vector<ParseNode> nodes_from_this_token_pack;
						const int old_consumed_tokens = consumed_tokens;
						for (const TokenType& token : token_pack.tokens) {
							ParseNode p;
							p.token.token_type = token;
							int ret = new_parse_node(&p, token_list, offset + consumed_tokens);
							if (ret == 0) {
								if (nodes_from_this_token_pack.size() != 0) {
									consumed_tokens = old_consumed_tokens;
									break;
								}
								// We did not match any (more) optionals so we're done
								break;
							} else {
								nodes_from_this_token_pack.push_back(p);
								consumed_tokens += ret;
							}
						}

						if (consumed_tokens != old_consumed_tokens) {
							for (const ParseNode& p : nodes_from_this_token_pack) {
								node->children.push_back(p);
							}
						}

						if (token_pack.rule_type == TokenPack::ZERO_OR_ONE ||  // if this rule type specifies only one extra
							consumed_tokens == old_consumed_tokens) { // or if we did not find any more extras
							break;
						}
					}
				}
			}

			// If we've gotten this far then we have matched the rule, return
			return consumed_tokens;

		next_rule:
			;
		}

	}

	// If we didn't match any of the rules
	return 0;
}

const static std::map<TokenType, RuleList> C_GRAMMAR = {
	{TokenType::translation_unit,
		{
			{{TokenType::external_declaration}, TokenType::external_declaration_tail},
			//{{TokenType::function}, TokenType::function_tail},
		}
	},
	{TokenType::external_declaration_tail,
		{
			{{TokenType::external_declaration}, {}},
		}
	},
	{TokenType::external_declaration,
		{
			{{TokenType::function}, {}},
			{{TokenType::declaration}, {}},
		}
	},
	{TokenType::function,
		{
			{{TokenType::function_declaration}, {}},
			{{TokenType::function_definition}, {}},
		}
	},
	{TokenType::function_declaration,
		{
			{{TokenType::declaration_specifier, TokenType::identifier, TokenType::open_parenth, TokenType::parameter_list, TokenType::close_parenth, TokenType::semi_colon}, {}},
			{{TokenType::declaration_specifier, TokenType::identifier, TokenType::open_parenth, TokenType::close_parenth, TokenType::semi_colon}, {}},
		}
	},
	{TokenType::function_definition,
		{
			{{TokenType::declaration_specifier, TokenType::identifier, TokenType::open_parenth, TokenType::parameter_list, TokenType::close_parenth, TokenType::compound_statement}, {}},
			{{TokenType::declaration_specifier, TokenType::identifier, TokenType::open_parenth, TokenType::close_parenth, TokenType::compound_statement}, {}},
		}
	},
	{TokenType::parameter_list,
		{
			{{TokenType::parameter_declaration}, TokenType::parameter_list_tail},
		}
	},
	{TokenType::parameter_list_tail,
		{
			{{TokenType::comma, TokenType::parameter_declaration}, {}},
		}
	},
	{TokenType::parameter_declaration,
		{
			{{TokenType::declaration_specifier, TokenType::init_declarator}, {}},
			{{TokenType::declaration_specifier}, {}},
		}
	},
	{TokenType::declaration_specifier,
		{
			{{TokenType::key_int}, {}},// TODO add other types and specifiers
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
			{{TokenType::declaration}, {}},
			{{TokenType::statement}, {}},
		}
	},
	{TokenType::declaration,
		{
			//{{TokenType::type_specifier, TokenType::semi_colon}, {}},// TODO for struct declaration
			{{TokenType::declaration_specifier, TokenType::init_declarator_list, TokenType::semi_colon}, {}},
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
			{{TokenType::declarator, TokenType::equals, TokenType::expression}, {}},
			{{TokenType::declarator}, {}},
		}
	},
	{TokenType::declarator,
		{
			{{TokenType::pointer, TokenType::direct_declarator}, {}},
			{{TokenType::direct_declarator}, {}},
		}
	},
	{TokenType::direct_declarator,
		{
			{{TokenType::identifier}, TokenType::direct_declarator_tail}, // var/pointer
		}
	},
	{TokenType::direct_declarator_tail,
		{
			{{TokenType::open_square_bracket, TokenType::constant, TokenType::close_square_bracket}, {}}, // technically should be const expression
		}
	},
	{TokenType::pointer,
		{
			{{TokenType::star}, TokenType::star},
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
			{{TokenType::unary_expression, TokenType::equals, TokenType::expression}, {}},
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
			{{TokenType::shift_exp}, TokenType::shift_exp_tail},
		}
	},
	{TokenType::shift_exp_tail,
		{
			{{TokenType::less_than, TokenType::shift_exp}, {}},
			{{TokenType::greater_than, TokenType::shift_exp}, {}},
			{{TokenType::le_op, TokenType::shift_exp}, {}},
			{{TokenType::ge_op, TokenType::shift_exp}, {}},
		}
	},
	{TokenType::shift_exp,
		{
			{{TokenType::additive_exp}, TokenType::additive_exp_tail},
		}
	},
	{ TokenType::additive_exp_tail,
		{
			{{TokenType::left_op, TokenType::additive_exp}, {}},
			{{TokenType::right_op, TokenType::additive_exp}, {}},
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
			{{TokenType::star, TokenType::factor}, {}}, // mult
			{{TokenType::div, TokenType::factor}, {}}, // div
			{{TokenType::percent, TokenType::factor}, {}}, // modulo
		}
	},
	{TokenType::factor,
		{
			{{TokenType::open_parenth, TokenType::expression, TokenType::close_parenth}, {}},
			{{TokenType::function_call}, {}},
			{{TokenType::unary_expression}, {}},
			{{TokenType::constant}, {}},
		}
	},
	{TokenType::function_call,
		{
			{{TokenType::identifier, TokenType::open_parenth, TokenType::close_parenth}, {}},
			{{TokenType::identifier, TokenType::open_parenth, TokenType::argument_expression_list, TokenType::close_parenth}, {}},
		}
	},
	{TokenType::argument_expression_list,
		{
			{{TokenType::expression}, TokenType::argument_expression_list_tail},
		}
	},
	{TokenType::argument_expression_list_tail,
		{
			{{TokenType::comma, TokenType::expression}, {}},
		}
	},
	{TokenType::unary_expression,
		{
			{{TokenType::sub, TokenType::factor}, {}},
			{{TokenType::tilda, TokenType::factor}, {}},
			{{TokenType::exclam, TokenType::factor}, {}},
			{{TokenType::ampersand, TokenType::identifier}, {}},
			{{TokenType::star, TokenType::identifier}, {}},
			{{TokenType::identifier, TokenType::open_square_bracket, TokenType::expression, TokenType::close_square_bracket}, {}},
			{{TokenType::identifier}, {}},
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
						// We actually want the children
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
	int tokens_parsed = new_parse_node(&root, token_list, 0);
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
