#include "AST.h"

namespace AST {

	std::shared_ptr<Statement> parse_statement(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::statement);

		const auto& child = node.children.at(0);

		// statements will only have a single child
		switch (child.token.token_type) {
		case TokenType::jump_statement:
		{
			if (child.children.at(0).token.token_type == TokenType::key_return) {
				return std::make_shared<Return_Statement>(child, scope);
			} else {
				throw std::logic_error("Invalid jump_statement");
			}
		}
		default:
			throw std::logic_error("Tried to parse_statement with invalid type: " +
				tokenType_to_string(child.token.token_type));
		}
	}

	Return_Statement::Return_Statement(const ParseNode& node, std::shared_ptr<Scope> scope)
			: Statement(scope) {
		node.check_type(TokenType::jump_statement);

		// Double check that this is actually a return
		if (!node.contains_child_with_type(TokenType::key_return)) {
			throw std::logic_error("Tried to construct return statement with something other than return statement");
		}

		// Return may optionally have an expression
		if (node.contains_child_with_type(TokenType::expression)) {
			ret_expression = parse_expression(node.get_child_with_type(TokenType::expression), scope);
		}
	}
}
