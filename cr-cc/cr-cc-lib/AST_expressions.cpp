#include "AST.h"

namespace AST {

	std::shared_ptr<Expression> parse_expression(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::expression);

		const auto& child = node.children.at(0);

		switch (child.token.token_type) {
		case TokenType::constant:
			return std::make_shared<Constant_Expression>(child, scope);
		case TokenType::unary_expression:
			return std::make_shared<Unary_Expression>(child, scope);
		default:
			throw std::logic_error("Tried to parse_expression with invalid expression type: " +
				tokenType_to_string(child.token.token_type));
		}
	}

	Unary_Expression::Unary_Expression(const ParseNode& node, std::shared_ptr<Scope> scope)
		: Expression(scope) {
		node.check_type(TokenType::unary_expression);

		// unary expression is always of the form <operator> <expression>
		const auto& the_operator = node.children.at(0);
		const auto& sub_expression = node.children.at(1);

		switch (the_operator.token.token_type) {
		case TokenType::sub:
			type = Type::negation;
			break;
		case TokenType::tilda:
			type = Type::bitwise_complement;
			break;
		case TokenType::exclam:
			type = Type::logical_negation;
			break;
		default:
			throw std::logic_error("Invalid unary token: " + tokenType_to_string(the_operator.token.token_type));
		}

		sub = parse_expression(sub_expression, scope);
	}

	Constant_Expression::Constant_Expression(const ParseNode& node, std::shared_ptr<Scope> scope)
			: Expression(scope) {
		node.check_type(TokenType::constant);
		int value = std::stoi(node.token.value);
		if (value > 0xFFFF || value < -0x7FFF) {
			throw std::logic_error("Contstant literal out of range: " + value);
		}
		constant_value = static_cast<std::uint16_t>(value);
	}
}
