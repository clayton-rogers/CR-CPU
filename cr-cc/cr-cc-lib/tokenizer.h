#pragma once

#include <string>
#include <vector>

#include "tokens.h"


struct Token {
	TokenType token_type = TokenType::NONE;
	std::string value;
};

using TokenList = std::vector<Token>;

TokenList tokenize(const std::string& code);

std::string print_token_list(TokenList tl);
