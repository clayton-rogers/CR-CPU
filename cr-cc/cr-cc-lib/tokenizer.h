#pragma once

#include <string>
#include <vector>

enum class TokenType {
	unk,

	user,

	key_struct,
	key_char,
	key_int,
	key_void,
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

	equals,
	add,
	sub,
	star,
	div,
	percent,
	ampersand,
	pipe,
	hat,
	period,
	comma,
	semi_colon,
	less_than,
	greater_than,

	open_bracket,
	close_bracket,
	open_parenth,
	close_parenth,
	open_square_bracket,
	close_square_bracket,
};

struct Token {
	std::string value;
	TokenType token_type = TokenType::unk;
};

using TokenList = std::vector<Token>;

TokenList tokenize(const std::string& code);

std::string print_tokens(TokenList tl);
