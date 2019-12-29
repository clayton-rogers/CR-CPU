#pragma once

#include <string>
#include <vector>

enum class TokenType {
	unk,

	key_struct,
	key_char,
	key_int,

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
