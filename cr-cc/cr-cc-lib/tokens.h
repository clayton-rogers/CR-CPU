#pragma once

#include <string>

enum class TokenType {
	// The default
	unk,

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
	function_definition,

	type_specifier,
	compound_statement,

	last_token,
};

std::string token_to_string(TokenType token);
