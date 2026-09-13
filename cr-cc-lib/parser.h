#pragma once

#include "tokenizer.h"

#include <string>
#include <vector>
#include <stdexcept>

struct ParseNode {
	Token token;
	std::vector<ParseNode> children;

	void check_type(TokenType type) const
	{
		if (token.token_type != type) {
			throw std::logic_error("AST: Node failed check type. Expected: " +
				tokenType_to_string(type) + " Actual: " + tokenType_to_string(token.token_type));
		}
	}

	const ParseNode& get_child_with_type(TokenType type) const
	{
		for (const auto& child : children) {
			if (child.token.token_type == type) {
				return child;
			}
		}
		throw std::logic_error(
			"AST: Could not find child of type: " + tokenType_to_string(type) +
			" on node of type " + tokenType_to_string(token.token_type));
	}

	bool contains_child_with_type(TokenType type) const
	{
		for (const auto& child : children) {
			if (child.token.token_type == type) {
				return true;
			}
		}

		return false;
	}
};

ParseNode parse(TokenList token_list);
