#pragma once

//#include "tokenizer.h"

#include <string>
#include <vector>

struct VariableType {
	std::string name;
	int size;
};

struct Declaration {
	bool is_const;
	VariableType type;
	std::string name;
};

enum class StatementSwitch {
	statement_list,
	actual_statement,
};

struct Statement {
	// One of:
	StatementSwitch statement_switch;
	// 1: Statement list:
	std::vector<Statement> statement_list;
	// 2: Actual statement:

};

struct Function {
	VariableType return_type;
	std::string name;
	std::vector<Declaration> arguments;
	Statement contents;
};

struct ParseTree {
	std::vector<Declaration> declarations;
	std::vector<Function> functions;
};

ParseTree parse(TokenList token_list);

#include <map>
#include <iostream> // TODO remove


using TokenTypeList = std::vector<TokenType>;
using TokenGrammar = std::vector<TokenTypeList>;


// ******************************************************** //
// C Language Grammar
// ******************************************************** //
static const std::map<TokenType, TokenGrammar> C_GRAMMAR = {
{TokenType::primary_expression, {
	{TokenType::identifier},
	{TokenType::constant},
	{TokenType::string_literal},
	{TokenType::open_parenth, TokenType::expression, TokenType::close_parenth},
	}},
{TokenType::postfix_expression, {
	{TokenType::primary_expression},
	{TokenType::postfix_expression, TokenType::open_square_bracket, TokenType::expression, TokenType::close_square_bracket},
	{TokenType::postfix_expression, TokenType::open_parenth, TokenType::close_parenth},
	{TokenType::postfix_expression, TokenType::period, TokenType::identifier},
	{TokenType::postfix_expression, TokenType::inc_op},
	{TokenType::postfix_expression, TokenType::dec_op},
	}},
{TokenType::argument_expression_list, {
	{TokenType::assignment_expression},
	{TokenType::argument_expression_list, TokenType::comma, TokenType::assignment_expression}
	}},
{TokenType::unary_expression,  {
	{TokenType::postfix_expression},
	{TokenType::inc_op, TokenType::unary_expression},
	{TokenType::dec_op, TokenType::unary_expression},
	{TokenType::unary_operator, TokenType::cast_expression},
	{TokenType::a_sizeof, TokenType::unary_expression},
	{TokenType::a_sizeof, TokenType::open_parenth, TokenType::type_name, TokenType::close_parenth},
	}},
{TokenType::unary_operator, {
	{TokenType::ampersand},
	{TokenType::star},
	{TokenType::add},
	{TokenType::sub},
	{TokenType::tilda},
	{TokenType::exclam},
	}},
{TokenType::cast_expression, {
	{TokenType::unary_expression},
	{TokenType::open_parenth, TokenType::type_name, TokenType::close_parenth, TokenType::cast_expression},
	}},
{TokenType::multiplicative_expression, {
	{TokenType::cast_expression},
	{TokenType::multiplicative_expression, TokenType::star, TokenType::cast_expression},
	{TokenType::multiplicative_expression, TokenType::div, TokenType::cast_expression},
	{TokenType::multiplicative_expression, TokenType::percent, TokenType::cast_expression},
	}},
{TokenType::additive_expression, {
	{TokenType::multiplicative_expression},
	{TokenType::additive_expression, TokenType::add, TokenType::multiplicative_expression},
	{TokenType::additive_expression, TokenType::sub, TokenType::multiplicative_expression},
	}},
{TokenType::shift_expression, {
	{TokenType::additive_expression},
	{TokenType::additive_expression, TokenType::add, TokenType::multiplicative_expression},
	{TokenType::additive_expression, TokenType::sub, TokenType::multiplicative_expression},
	}},
{TokenType::relational_expression,  {
	{TokenType::shift_expression},
	{TokenType::relational_expression, TokenType::less_than, TokenType::shift_expression},
	{TokenType::relational_expression, TokenType::greater_than, TokenType::shift_expression},
	{TokenType::relational_expression, TokenType::le_op, TokenType::shift_expression},
	{TokenType::relational_expression, TokenType::ge_op, TokenType::shift_expression},
	}},
{TokenType::equality_expression, {
	{TokenType::relational_expression},
	{TokenType::equality_expression, TokenType::eq_op, TokenType::relational_expression},
	{TokenType::equality_expression, TokenType::ne_op, TokenType::relational_expression},
	}},
{TokenType::and_expression, {
	{TokenType::equality_expression},
	{TokenType::and_expression, TokenType::ampersand, TokenType::equality_expression},
	}},
{TokenType::exclusive_or_expression, {
	{TokenType::and_expression},
	{TokenType::exclusive_or_expression, TokenType::hat, TokenType::and_expression},
	}},
{TokenType::inclusive_or_expression, {
	{TokenType::exclusive_or_expression},
	{TokenType::inclusive_or_expression, TokenType::pipe, TokenType::exclusive_or_expression},
	}},
{TokenType::logical_and_expression, {
	{TokenType::inclusive_or_expression},
	{TokenType::logical_and_expression, TokenType::and_op, TokenType::inclusive_or_expression},
	}},
{TokenType::logical_or_expression, {
	{TokenType::logical_and_expression},
	{TokenType::logical_or_expression, TokenType::or_op, TokenType::logical_and_expression},
	}},
{TokenType::conditional_expression, {
	{TokenType::logical_or_expression},
	{TokenType::logical_or_expression, TokenType::question, TokenType::expression, TokenType::colon, TokenType::conditional_expression},
	}},
{TokenType::assignment_expression, {
	{TokenType::conditional_expression},
	{TokenType::unary_expression, TokenType::assignemnt_operator, TokenType::assignment_expression},
	}},
{TokenType::assignemnt_operator, {
	{TokenType::equals},
	{TokenType::star, TokenType::equals},
	{TokenType::div, TokenType::equals},
	{TokenType::percent, TokenType::equals},
	{TokenType::add, TokenType::equals},
	{TokenType::sub, TokenType::equals},
	{TokenType::less_than, TokenType::less_than, TokenType::equals},
	{TokenType::greater_than, TokenType::greater_than, TokenType::equals},
	{TokenType::ampersand, TokenType::equals},
	{TokenType::hat, TokenType::equals},
	{TokenType::pipe, TokenType::equals},
	}},
{TokenType::expression, {
	{TokenType::assignment_expression},
	{TokenType::expression, TokenType::comma, TokenType::assignment_expression},
	}},
{TokenType::constant_expression, {
	{TokenType::conditional_expression},
	}},
{TokenType::declaration, {
	{TokenType::declaration_specifiers, TokenType::semi_colon},
	{TokenType::declaration_specifiers, TokenType::init_declarator_list, TokenType::semi_colon},
	}},
{TokenType::declaration_specifiers, {
	{TokenType::storage_class_specifier},
	{TokenType::storage_class_specifier, TokenType::declaration_specifiers},
	{TokenType::type_specifier},
	{TokenType::type_specifier, TokenType::declaration_specifiers},
	{TokenType::type_qualifier},
	{TokenType::type_qualifier, TokenType::declaration_specifiers},
	}},
{TokenType::init_declarator_list, {
	{TokenType::init_declarator},
	{TokenType::init_declarator_list, TokenType::comma, TokenType::init_declarator},
	}},
{TokenType::init_declarator, {
	{TokenType::declarator},
	{TokenType::declarator, TokenType::equals, TokenType::initializer},
	}},
{TokenType::storage_class_specifier, {
	{TokenType::key_typedef},
	{TokenType::key_extern},
	{TokenType::key_static},
	{TokenType::key_auto},
	{TokenType::key_register},
	}},
{TokenType::type_specifier, {
	{TokenType::key_void},
	{TokenType::key_char},
	{TokenType::key_short},
	{TokenType::key_int},
	{TokenType::key_long},
	{TokenType::key_float},
	{TokenType::key_double},
	{TokenType::key_signed},
	{TokenType::key_unsigned},
	{TokenType::struct_or_union_specifier},
	{TokenType::enum_specifier},
	{TokenType::identifier}, // TYPE_NAME as per standard
	}},
{TokenType::struct_or_union_specifier, {
	{TokenType::struct_or_union, TokenType::identifier, TokenType::open_bracket, TokenType::struct_declaration_list, TokenType::close_bracket},
	{TokenType::struct_or_union, TokenType::open_bracket, TokenType::struct_declaration_list, TokenType::close_bracket},
	{TokenType::struct_or_union, TokenType::identifier},
	}},
{TokenType::struct_or_union, {
	{TokenType::key_struct},
	{TokenType::key_union},
	}},
{TokenType::struct_declaration_list, {
	{TokenType::struct_declaration},
	{TokenType::struct_declaration_list, TokenType::struct_declaration},
	}},
{TokenType::struct_declaration, {
	{TokenType::specifier_qualifier_list, TokenType::struct_declarator_list, TokenType::semi_colon},
	}},
{TokenType::specifier_qualifier_list, {
	{TokenType::type_specifier, TokenType::specifier_qualifier_list},
	{TokenType::type_specifier},
	{TokenType::type_qualifier, TokenType::specifier_qualifier_list},
	{TokenType::type_qualifier},
	}},
{TokenType::struct_declarator_list, {
	{TokenType::struct_declarator},
	{TokenType::struct_declarator_list, TokenType::comma, TokenType::struct_declarator},
	}},
{TokenType::struct_declarator, {
	{TokenType::declarator},
	{TokenType::colon, TokenType::constant_expression},
	{TokenType::declarator, TokenType::colon, TokenType::constant_expression},
	}},
{TokenType::enum_specifier, {
	{TokenType::key_enum, TokenType::open_bracket, TokenType::enumerator_list, TokenType::close_bracket},
	{TokenType::key_enum, TokenType::identifier, TokenType::open_bracket, TokenType::enumerator_list, TokenType::close_bracket},
	{TokenType::key_enum, TokenType::identifier},
	}},
{TokenType::enumerator_list, {
	{TokenType::enumerator},
	{TokenType::enumerator_list, TokenType::comma, TokenType::enumerator},
	}},
{TokenType::enumerator, {
	{TokenType::identifier},
	{TokenType::identifier, TokenType::equals, TokenType::constant_expression},
	}},
{TokenType::type_qualifier, {
	{TokenType::key_const},
	{TokenType::key_volatile},
	}},
{TokenType::declarator, {
	{TokenType::pointer, TokenType::direct_declarator},
	{TokenType::direct_declarator},
	}},
{TokenType::direct_declarator, {
	{TokenType::identifier},
	{TokenType::open_parenth, TokenType::declarator, TokenType::close_parenth},
	{TokenType::direct_declarator, TokenType::open_square_bracket, TokenType::constant_expression, TokenType::close_square_bracket},
	{TokenType::direct_declarator, TokenType::open_square_bracket, TokenType::close_square_bracket},
	{TokenType::direct_declarator, TokenType::open_parenth, TokenType::parameter_type_list, TokenType::close_parenth},
	{TokenType::direct_declarator, TokenType::open_parenth, TokenType::identifier_list, TokenType::close_parenth},
	{TokenType::direct_declarator, TokenType::open_parenth, TokenType::close_parenth},
	}},
{TokenType::pointer, {
	{TokenType::star},
	{TokenType::star, TokenType::type_qualifier_list},
	{TokenType::star, TokenType::pointer},
	{TokenType::star, TokenType::type_qualifier_list, TokenType::pointer},
	}},
{TokenType::type_qualifier_list, {
	{TokenType::type_qualifier},
	{TokenType::type_qualifier_list, TokenType::type_qualifier},
	}},
{TokenType::parameter_type_list, {
	{TokenType::parameter_list},
	{TokenType::parameter_list, TokenType::comma, TokenType::period, TokenType::period, TokenType::period}, // elipses
	}},
{TokenType::parameter_list, {
	{TokenType::parameter_declaration},
	{TokenType::parameter_list, TokenType::comma, TokenType::parameter_declaration},
	}},
{TokenType::parameter_declaration, {
	{TokenType::declaration_specifiers, TokenType::declarator},
	{TokenType::declaration_specifiers, TokenType::abstract_declarator},
	{TokenType::declaration_specifiers},
	}},
{TokenType::identifier_list, {
	{TokenType::identifier},
	{TokenType::identifier_list, TokenType::comma, TokenType::identifier},
	}},
{TokenType::type_name, {
	{TokenType::specifier_qualifier_list},
	{TokenType::specifier_qualifier_list, TokenType::abstract_declarator},
	}},
{TokenType::abstract_declarator, {
	{TokenType::pointer},
	{TokenType::direct_abstract_declarator},
	{TokenType::pointer, TokenType::direct_abstract_declarator},
	}},
{TokenType::direct_abstract_declarator, {
	{TokenType::open_parenth, TokenType::abstract_declarator, TokenType::close_parenth},
	{TokenType::open_square_bracket, TokenType::close_square_bracket},
	{TokenType::open_square_bracket, TokenType::constant_expression, TokenType::close_square_bracket},
	{TokenType::direct_abstract_declarator, TokenType::open_square_bracket, TokenType::close_square_bracket},
	{TokenType::direct_abstract_declarator, TokenType::open_square_bracket, TokenType::constant_expression, TokenType::close_square_bracket},
	{TokenType::open_parenth, TokenType::close_parenth},
	{TokenType::open_parenth, TokenType::parameter_type_list, TokenType::close_parenth},
	{TokenType::direct_abstract_declarator, TokenType::open_parenth, TokenType::close_parenth},
	{TokenType::direct_abstract_declarator, TokenType::open_parenth, TokenType::parameter_type_list, TokenType::close_parenth},
	}},
{TokenType::initializer, {
	{TokenType::assignment_expression},
	{TokenType::open_bracket, TokenType::initializer_list, TokenType::close_bracket},
	{TokenType::open_bracket, TokenType::initializer_list, TokenType::comma, TokenType::close_bracket},
	}},
{TokenType::initializer_list, {
	{TokenType::initializer},
	{TokenType::initializer_list, TokenType::comma, TokenType::initializer},
	}},
{TokenType::statement, {
	{TokenType::labeled_statement},
	{TokenType::compound_statement},
	{TokenType::expression_statement},
	{TokenType::selection_statement},
	{TokenType::iteration_statement},
	{TokenType::jump_statement},
	}},
{TokenType::labeled_statement, {
	{TokenType::identifier, TokenType::colon, TokenType::statement},
	{TokenType::key_case, TokenType::constant_expression, TokenType::colon, TokenType::statement},
	{TokenType::key_default, TokenType::colon, TokenType::statement},
	}},
{TokenType::compound_statement, {
	{TokenType::open_bracket, TokenType::close_bracket},
	{TokenType::open_bracket, TokenType::statement_list, TokenType::close_bracket},
	{TokenType::open_bracket, TokenType::declaration_list, TokenType::close_bracket},
	{TokenType::open_bracket, TokenType::declaration_list, TokenType::statement_list, TokenType::close_bracket}
	}},
{TokenType::declaration_list, {
	{TokenType::declaration},
	{TokenType::declaration_list, TokenType::declaration},
	}},
{TokenType::statement_list, {
	{TokenType::statement},
	{TokenType::statement_list, TokenType::statement},
	}},
{TokenType::expression_statement, {
	{TokenType::semi_colon},
	{TokenType::expression, TokenType::semi_colon},
	}},
{TokenType::selection_statement, {
	{TokenType::key_if, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::statement},
	{TokenType::key_if, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::statement, TokenType::key_else, TokenType::statement},
	{TokenType::key_switch, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::statement},
	}},
{TokenType::iteration_statement, {
	{TokenType::key_while, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::statement},
	{TokenType::key_do, TokenType::statement, TokenType::key_while, TokenType::open_parenth, TokenType::expression, TokenType::close_parenth, TokenType::semi_colon},
	{TokenType::key_for, TokenType::open_parenth, TokenType::expression_statement, TokenType::expression_statement, TokenType::close_parenth, TokenType::statement},
	{TokenType::key_for, TokenType::open_parenth, TokenType::expression_statement, TokenType::expression_statement, TokenType::expression, TokenType::close_parenth, TokenType::statement},
	}},
{TokenType::jump_statement, {
	{TokenType::key_goto, TokenType::identifier, TokenType::semi_colon},
	{TokenType::key_continue, TokenType::semi_colon},
	{TokenType::key_break, TokenType::semi_colon},
	{TokenType::key_return, TokenType::semi_colon},
	{TokenType::key_return, TokenType::expression, TokenType::semi_colon},
	}},
{TokenType::translation_unit, {
	{TokenType::external_declaration},
	{TokenType::translation_unit, TokenType::external_declaration},
	}},
{TokenType::external_declaration, {
	{TokenType::function_definition},
	{TokenType::declaration},
	}},
{TokenType::function_definition, {
	{TokenType::declaration_specifiers, TokenType::declarator, TokenType::declaration_list, TokenType::compound_statement},
	{TokenType::declaration_specifiers, TokenType::declarator, TokenType::compound_statement},
	{TokenType::declarator, TokenType::declaration_list, TokenType::compound_statement},
	{TokenType::declarator, TokenType::compound_statement},
	}},

	// These are not specified by the formal grammar of C, I've just added them since my tokenizer separates them
	{TokenType::right_assign, {
		{TokenType::greater_than, TokenType::greater_than, TokenType::equals},
		}},
	{TokenType::left_assign, {
		{TokenType::less_than, TokenType::less_than, TokenType::equals},
		}},
	{TokenType::add_assign, {
		{TokenType::add, TokenType::equals},
		}},
	{TokenType::sub_assign, {
		{TokenType::sub, TokenType::equals},
		}},
	{TokenType::mul_assign, {
		{TokenType::star, TokenType::equals},
		}},
	{TokenType::div_assign, {
		{TokenType::div, TokenType::equals},
		}},
	{TokenType::mod_assign, {
		{TokenType::percent, TokenType::equals},
		}},
	{TokenType::and_assign, {
		{TokenType::ampersand, TokenType::equals},
		}},
	{TokenType::xor_assign, {
		{TokenType::hat, TokenType::equals},
		}},
	{TokenType::or_assign, {
		{TokenType::pipe, TokenType::equals},
		}},
	{TokenType::right_op, {
		{TokenType::less_than, TokenType::less_than},
		}},
	{TokenType::left_op, {
		{TokenType::greater_than, TokenType::greater_than},
		}},
	{TokenType::inc_op, {
		{TokenType::add, TokenType::add},
		}},
	{TokenType::dec_op, {
		{TokenType::sub, TokenType::sub},
		}},
	{TokenType::ptr_op, {
		{TokenType::sub, TokenType::greater_than},
		}},
	{TokenType::and_op, {
		{TokenType::ampersand, TokenType::ampersand},
		}},
	{TokenType::or_op, {
		{TokenType::pipe, TokenType::pipe},
		}},
	{TokenType::le_op, {
		{TokenType::less_than, TokenType::equals},
		}},
	{TokenType::ge_op, {
		{TokenType::greater_than, TokenType::equals},
		}},
	{TokenType::eq_op, {
		{TokenType::equals, TokenType::equals},
		}},
	{TokenType::ne_op, {
		{TokenType::exclam, TokenType::equals},
		}},
};

struct ParserNode {
	TokenType token_type = TokenType::unk;
	Token token;
	std::vector<ParserNode> children;
};

static bool is_token_terminal(TokenType token) {
	// A token is terminal when it can no longer be broken into more tokens,
	// in this case the token will not appear in the grammar and will have been
	// recognized by the tokenizer.
	return C_GRAMMAR.count(token) != 1;
}

// Returns the number of tokens consumed
static int parse_node(ParserNode* node, TokenList token_list, int offset) {
	if (is_token_terminal(node->token_type)) {
		if (node->token_type == token_list.at(offset).token_type) {
			node->token = token_list.at(offset);
			return 1; // parse was successful
		} else {
			return 0; // no match
		}
	} else {
		TokenGrammar g = C_GRAMMAR.at(node->token_type);

		// Each grammar rule has potentially multiple patterns that will satisfy it
		for (const auto& option : g) {
			// In a given pattern there may be one or more tokens
			int count_of_consumed_tokens = 0;
			for (const auto& token_type : option) {
				ParserNode p;
				p.token_type = token_type;
				int ret = parse_node(&p, token_list, offset + count_of_consumed_tokens);
				if (ret == 0) {
					count_of_consumed_tokens = 0;
					node->children.clear();
					break;
				} else {
					node->children.push_back(p);
					count_of_consumed_tokens += ret;
				}
			}
			// If we pass all the tokens in a given option, then we are good
			if (count_of_consumed_tokens != 0) {
				return count_of_consumed_tokens;
			}
		}

		// If we've exhausted all the options and not found a match then this branch is dead
		return 0;
	}
}

ParseTree parse(TokenList token_list) {

	std::cout << "Following tokens are considered terminal:" << std::endl;
	for (int i = 0; i < (int)TokenType::last_token; ++i) {
		TokenType token_type = (TokenType)i;
		if (C_GRAMMAR.count(token_type) != 1) {
			std::cout << token_to_string(token_type) << std::endl;
		}
	}

	ParserNode top;
	top.token_type = TokenType::translation_unit;
	int tokens_parsed = parse_node(&top, token_list, 0);
	if (tokens_parsed == 0) {
		throw std::logic_error("Failed to parse");
	} else {
		std::cout << "Parsed " << tokens_parsed << " tokens!!!" << std::endl;
	}
	return ParseTree();
}
