#pragma once

#include <string>
#include <vector>

#include "tokens.h"


struct Token {
	std::string value;
	TokenType token_type = TokenType::unk;
};

using TokenList = std::vector<Token>;

TokenList tokenize(const std::string& code);

std::string print_token_list(TokenList tl);
