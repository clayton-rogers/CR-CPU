#include "AST.h"

namespace AST {

	static std::shared_ptr<Expression> parse_factor(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::factor);

		auto child = node.children.at(0);
		switch (child.token.token_type) {
		case TokenType::constant:
			return std::make_shared<Constant_Expression>(child, scope);
		case TokenType::unary_expression:
			return std::make_shared<Unary_Expression>(child, scope);
		case TokenType::open_parenth:
			// ( exp )
			return parse_expression(node.children.at(1), scope);
		default:
			throw std::logic_error("Tried to parse_factor with invalid expression type: " +
				tokenType_to_string(child.token.token_type));
		}
	}

	static std::shared_ptr<Expression> parse_term(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::term);

		// A term may be
		// factor
		// factor * factor
		// factor * factor * factor
		if (node.children.size() % 2 != 1) {
			throw std::logic_error("Term should always have an even number of terms");
		}

		// In the case of multiple factors, we make them left associative
		//          /*\ <= root
		//      /*\    factor
		// factor  factor
		std::vector<std::shared_ptr<Expression>> exps;
		std::vector<TokenType> operators;
		for (int i = 0; i < static_cast<int>(node.children.size()); i += 2) {
			if (i != 0) {
				operators.push_back(node.children.at(i - 1).token.token_type);
			}
			exps.push_back(parse_factor(node.children.at(i), scope));
		}

		std::shared_ptr<Expression> left = exps.front();
		exps.erase(exps.begin());
		while (!exps.empty()) {
			std::shared_ptr<Expression> right = exps.front();
			exps.erase(exps.begin());

			TokenType the_operator = operators.front();
			operators.erase(operators.begin());

			std::shared_ptr<Expression> new_left =
				std::make_shared<Binary_Expression>(the_operator, left, right, scope);
			left = new_left;
		}

		return left;
	}

	std::shared_ptr<Expression> parse_expression(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::expression);

		// An expression may be
		// term
		// term + term
		// term + term + term .....

		if (node.children.size() % 2 != 1) {
			throw std::logic_error("Expression should always have an even number of terms");
		}

		// In the case of multiple terms, we make them left associative
		//        /*\  <= root
		//     /*\    term
		// term  term
		std::vector<std::shared_ptr<Expression>> exps;
		std::vector<TokenType> operators;
		for (int i = 0; i < static_cast<int>(node.children.size()); i += 2) {
			if (i != 0) {
				operators.push_back(node.children.at(i - 1).token.token_type);
			}
			exps.push_back(parse_term(node.children.at(i), scope));
		}

		std::shared_ptr<Expression> left = exps.front();
		exps.erase(exps.begin());
		while (!exps.empty()) {
			std::shared_ptr<Expression> right = exps.front();
			exps.erase(exps.begin());

			TokenType the_operator = operators.front();
			operators.erase(operators.begin());

			std::shared_ptr<Expression> new_left =
				std::make_shared<Binary_Expression>(the_operator, left, right, scope);
			left = new_left;
		}

		return left;
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

		sub = parse_factor(sub_expression, scope);
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

	Binary_Expression::Type Binary_Expression::token_to_type(TokenType type) {
		switch (type) {
		case TokenType::add:
			return Type::addition;
		case TokenType::sub:
			return Type::subtraction;
		case TokenType::star:
			return Type::multiplication;
		case TokenType::div:
			return Type::division;
		default:
			throw std::logic_error("Tried to convert invalid TokenType to binary expression: "
				+ tokenType_to_string(type));
		}
	}
}
