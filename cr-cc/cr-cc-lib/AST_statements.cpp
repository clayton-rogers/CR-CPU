#include "AST.h"

namespace AST {

	std::shared_ptr<Statement> parse_statement(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::statement);

		const auto& child = node.children.at(0);

		// statements will only have a single child
		switch (child.token.token_type) {
		case TokenType::compound_statement:
			return std::make_shared<Compount_Statement>(child, scope);
		case TokenType::jump_statement:
			if (child.children.at(0).token.token_type == TokenType::key_return) {
				return std::make_shared<Return_Statement>(child, scope);
			} else {
				throw std::logic_error("Invalid jump_statement");
			}
		case TokenType::expression_statement:
			return std::make_shared<Expression_Statement>(child, scope);
		default:
			throw std::logic_error("Tried to parse_statement with invalid type: " +
				tokenType_to_string(child.token.token_type));
		}
	}

	std::vector<std::shared_ptr<Statement>> parse_declaration(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::declaration);

		std::vector<std::shared_ptr<Statement>> ret;

		const Type* type_of_dec = parse_type(node.get_child_with_type(TokenType::type_specifier), scope);

		const auto& declarator_list = node.get_child_with_type(TokenType::init_declarator_list);
		for (const auto& declarator : declarator_list.children) {
			switch (declarator.token.token_type) {
			case TokenType::init_declarator:
			{
				// Add the var to the current scope
				std::string name = declarator.get_child_with_type(TokenType::identifier).token.value;
				scope->create_stack_var(type_of_dec, name);

				// If the var has an initializer, create a fake expression statement for it
				if (declarator.contains_child_with_type(TokenType::equals)) {
					auto init_exp = parse_expression(declarator.get_child_with_type(TokenType::expression), scope);
					auto assign_exp = std::make_shared<Assignment_Expression>(name, init_exp, scope);
					ret.push_back(std::make_shared<Expression_Statement>(assign_exp, scope));
				}
			}
			break;
			case TokenType::comma: // commas are expected
				break;
			default:
				throw std::logic_error("parse_declaration got invalid declarator: "
					+ tokenType_to_string(declarator.token.token_type));
			}
		}

		return ret;
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

	Expression_Statement::Expression_Statement(const ParseNode& node, std::shared_ptr<Scope> scope)
		: Statement(scope) {
		node.check_type(TokenType::expression_statement);

		// Expression statement is an expression followed by a semi colon
		sub = parse_expression(node.children.at(0), scope);
	}
}
