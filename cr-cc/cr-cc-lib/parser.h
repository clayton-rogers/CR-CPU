#pragma once

#include "tokenizer.h"

#include <string>


struct ParseNode {
	Token token;
	TokenList children;
};

ParseNode parse(TokenList token_list);
