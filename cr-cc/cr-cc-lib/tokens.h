#pragma once

#include <string>

enum class TokenType {
	// The default
	NONE,

	// Tokens recognized by the tokenizer
	key_struct,
	key_union,
	key_void,
	key_char,
	key_short,
	key_int,
	key_long,
	key_float,
	key_double,
	key_signed,
	key_unsigned,
	key_return,
	key_case,
	key_break,
	key_continue,
	key_default,
	key_do,
	key_else,
	key_enum,
	key_for,
	key_goto,
	key_if,
	key_switch,
	key_while,
	key_const,
	key_volatile,
	key_typedef,
	key_extern,
	key_static,
	key_auto,
	key_register,

	equals,
	add,
	sub,
	star,
	tilda,
	exclam,
	div,
	percent,
	ampersand,
	pipe,
	question,
	hat,
	period,
	comma,
	semi_colon,
	colon,
	less_than,
	greater_than,

	open_bracket,
	close_bracket,
	open_parenth,
	close_parenth,
	open_square_bracket,
	close_square_bracket,

	identifier,
	constant,
	string_literal,
	a_sizeof,
	ptr_op,
	inc_op,
	dec_op,
	left_op,
	right_op,
	le_op,
	ge_op,
	eq_op,
	ne_op,
	and_op,
	or_op,
	mul_assign,
	div_assign,
	mod_assign,
	add_assign,
	sub_assign,
	left_assign, // NEVER recognized, don't use
	right_assign,// NEVER recognized, don't use
	and_assign,
	xor_assign,
	or_assign,


	// Tokens recognized by the parser
	translation_unit,
	function,
	function_declaration,
	function_definition,
	parameter_list,
	parameter_list_tail,
	parameter_declaration,
	type_specifier,
	compound_statement,
	block_item,
	block_item_list,
	statement,
	declaration,
	init_declarator_list,
	init_declarator_list_tail,
	init_declarator,
	jump_statement,
	if_statement,
	while_statement,
	do_while_statement,
	for_statement,
	expression_statement,
	break_statement,
	continue_statement,
	expression,
	for_init_expression,
	for_condition_expression,
	for_increment_expression,
	conditional_exp,
	conditional_exp_tail,
	logical_or_exp,
	logical_or_exp_tail,
	logical_and_exp,
	logical_and_exp_tail,
	equality_exp,
	equality_exp_tail,
	relational_exp,
	relational_exp_tail,
	additive_exp,
	additive_exp_tail,
	shift_exp,
	shift_exp_tail,
	unary_expression,
	term,
	term_tail,
	factor,
	factor_tail,
	function_call,
	argument_expression_list,
	argument_expression_list_tail,
	external_declaration,
	external_declaration_tail,
	pointer,

	last_token,
};

std::string tokenType_to_string(TokenType token);
