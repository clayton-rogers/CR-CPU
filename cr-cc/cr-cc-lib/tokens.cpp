#include "tokens.h"

#include <map>

static const std::map<TokenType, std::string> TOKEN_STRINGS =
{
	{TokenType::NONE, "NONE"},
	{TokenType::key_struct, "key_struct"},
	{TokenType::key_union, "key_union"},
	{TokenType::key_void, "key_void"},
	{TokenType::key_char, "key_char"},
	{TokenType::key_short, "key_short"},
	{TokenType::key_int, "key_int"},
	{TokenType::key_long, "key_long"},
	{TokenType::key_float, "key_float"},
	{TokenType::key_double, "key_double"},
	{TokenType::key_signed, "key_signed"},
	{TokenType::key_unsigned, "key_unsigned"},
	{TokenType::key_return, "key_return"},
	{TokenType::key_case, "key_case"},
	{TokenType::key_break, "key_break"},
	{TokenType::key_continue, "key_continue"},
	{TokenType::key_default, "key_default"},
	{TokenType::key_do, "key_do"},
	{TokenType::key_else, "key_else"},
	{TokenType::key_enum, "key_enum"},
	{TokenType::key_for, "key_for"},
	{TokenType::key_goto, "key_goto"},
	{TokenType::key_if, "key_if"},
	{TokenType::key_switch, "key_switch"},
	{TokenType::key_while, "key_while"},
	{TokenType::key_const, "key_const"},
	{TokenType::key_volatile, "key_volatile"},
	{TokenType::key_typedef, "key_typedef"},
	{TokenType::key_extern, "key_extern"},
	{TokenType::key_static, "key_static"},
	{TokenType::key_auto, "key_auto"},
	{TokenType::key_register, "key_register"},
	{TokenType::equals, "equals"},
	{TokenType::add, "add"},
	{TokenType::sub, "sub"},
	{TokenType::star, "star"},
	{TokenType::tilda, "tilda"},
	{TokenType::exclam, "exclam"},
	{TokenType::div, "div"},
	{TokenType::percent, "percent"},
	{TokenType::ampersand, "ampersand"},
	{TokenType::pipe, "pipe"},
	{TokenType::question, "question"},
	{TokenType::hat, "hat"},
	{TokenType::period, "period"},
	{TokenType::comma, "comma"},
	{TokenType::semi_colon, "semi_colon"},
	{TokenType::colon, "colon"},
	{TokenType::less_than, "less_than"},
	{TokenType::greater_than, "greater_than"},
	{TokenType::open_bracket, "open_bracket"},
	{TokenType::close_bracket, "close_bracket"},
	{TokenType::open_parenth, "open_parenth"},
	{TokenType::close_parenth, "close_parenth"},
	{TokenType::open_square_bracket, "open_square_bracket"},
	{TokenType::close_square_bracket, "close_square_bracket"},
	{TokenType::identifier, "identifier"},
	{TokenType::constant, "constant"},
	{TokenType::string_literal, "string_literal"},
	{TokenType::a_sizeof, "a_sizeof"},
	{TokenType::ptr_op, "ptr_op"},
	{TokenType::inc_op, "inc_op"},
	{TokenType::dec_op, "dec_op"},
	{TokenType::left_op, "left_op"},
	{TokenType::right_op, "right_op"},
	{TokenType::le_op, "le_op"},
	{TokenType::ge_op, "ge_op"},
	{TokenType::eq_op, "eq_op"},
	{TokenType::ne_op, "ne_op"},
	{TokenType::and_op, "and_op"},
	{TokenType::or_op, "or_op"},
	{TokenType::mul_assign, "mul_assign"},
	{TokenType::div_assign, "div_assign"},
	{TokenType::mod_assign, "mod_assign"},
	{TokenType::add_assign, "add_assign"},
	{TokenType::sub_assign, "sub_assign"},
	{TokenType::left_assign, "left_assign"},
	{TokenType::right_assign, "right_assign"},
	{TokenType::and_assign, "and_assign"},
	{TokenType::xor_assign, "xor_assign"},
	{TokenType::or_assign, "or_assign"},
	{TokenType::translation_unit, "translation_unit"},
	{TokenType::function_definition, "function_definition"},
	{TokenType::type_specifier, "type_specifier"},
	{TokenType::compound_statement, "compound_statement"},
	{TokenType::statement_list, "statement_list"},
	{TokenType::statement, "statement"},
	{TokenType::jump_statement, "jump_statement"},
	{TokenType::expression, "expression"},
	{TokenType::last_token, "last_token"},
};

std::string tokenType_to_string(TokenType token) {
	return TOKEN_STRINGS.at(token);
}
