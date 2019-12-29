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

	key_const,

	equals,
	add,
	sub,
	star,
	div,
	comma,
	semi_colon,
	less_than,
	greater_than,

	open_bracket,
	close_bracket,
	open_parenth,
	close_parenth,
};

struct Token {
	std::string value;
	TokenType token_type = TokenType::unk;
};

using TokenList = std::vector<Token>;

TokenList tokenize(const std::string& code);

std::string print_tokens(TokenList tl);
