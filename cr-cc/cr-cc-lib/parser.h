#pragma once

#include "tokenizer.h"

#include <string>
#include <vector>


struct ParseNode {
	Token token;
	std::vector<ParseNode> children;
};

ParseNode parse(TokenList token_list);
