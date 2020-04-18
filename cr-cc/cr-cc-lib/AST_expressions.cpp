#include "AST.h"

namespace AST {

	static std::shared_ptr<Expression> parse_factor(const ParseNode& node, std::shared_ptr<VarMap> scope) {
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
		case TokenType::identifier:
			return std::make_shared<Variable_Expression>(child, scope);
		case TokenType::function_call:
			return std::make_shared<Function_Call_Expression>(child, scope);
		default:
			throw std::logic_error("Tried to parse_factor with invalid expression type: " +
				tokenType_to_string(child.token.token_type));
		}
	}

	static std::shared_ptr<Expression> parse_binary(const ParseNode& node, std::shared_ptr<VarMap> scope) {
		switch (node.token.token_type) {
			// If this is one of the binary sub expressions we're good
		case TokenType::term:
		case TokenType::additive_exp:
		case TokenType::relational_exp:
		case TokenType::equality_exp:
		case TokenType::logical_and_exp:
		case TokenType::logical_or_exp:
		case TokenType::expression:
			break;
		case TokenType::factor:
			return parse_factor(node, scope);
		default:
			throw std::logic_error("Tried to parse unknown token as binary exp: " + tokenType_to_string(node.token.token_type));
		}

		// A binary may be
		// item
		// item op item
		// item op item op item
		if (node.children.size() % 2 != 1) {
			throw std::logic_error("Binary expression should always have an even number of terms");
		}

		// In the case of multiple item, we make them left associative
		//        /*\ <= root
		//     /*\    item
		// item  item
		std::vector<std::shared_ptr<Expression>> exps;
		std::vector<TokenType> operators;
		for (int i = 0; i < static_cast<int>(node.children.size()); i += 2) {
			if (i != 0) {
				operators.push_back(node.children.at(i - 1).token.token_type);
			}
			exps.push_back(parse_binary(node.children.at(i), scope));
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

	std::shared_ptr<Expression> parse_conditional(const ParseNode& node, std::shared_ptr<VarMap> scope) {
		node.check_type(TokenType::conditional_exp);

		if (node.contains_child_with_type(TokenType::question)) {
			return std::make_shared<Conditional_Expression>(node, scope);
		} else {
			// Just passing through, no actual conditional
			return parse_binary(node.children.at(0), scope);
		}
	}

	std::shared_ptr<Expression> parse_expression(const ParseNode& node, std::shared_ptr<VarMap> scope) {
		node.check_type(TokenType::expression);

		if (node.contains_child_with_type(TokenType::equals)) {
			// this is an assigment
			return std::make_shared<Assignment_Expression>(node, scope);
		} else {
			return parse_conditional(node.get_child_with_type(TokenType::conditional_exp), scope);
		}
	}

	Assignment_Expression::Assignment_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Expression(scope) {
		node.check_type(TokenType::expression);

		var_name = node.get_child_with_type(TokenType::identifier).token.value;
		exp = parse_expression(node.get_child_with_type(TokenType::expression), scope);
	}

	Unary_Expression::Unary_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
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

	Constant_Expression::Constant_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
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
		case TokenType::and_op:
			return Type::logical_and;
		case TokenType::or_op:
			return Type::logical_or;
		case TokenType::eq_op:
			return Type::equal;
		case TokenType::ne_op:
			return Type::not_equal;
		case TokenType::less_than:
			return Type::less_than;
		case TokenType::le_op:
			return Type::less_than_or_equal;
		case TokenType::greater_than:
			return Type::greater_than;
		case TokenType::ge_op:
			return Type::greater_than_or_equal;
		default:
			throw std::logic_error("Tried to convert invalid TokenType to binary expression: "
				+ tokenType_to_string(type));
		}
	}

	Variable_Expression::Variable_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Expression(scope) {
		node.check_type(TokenType::identifier);

		var_name = node.token.value;
	}

	Conditional_Expression::Conditional_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Expression(scope) {
		node.check_type(TokenType::conditional_exp);
		if (!node.contains_child_with_type(TokenType::question)) {
			throw std::logic_error("Called conditional_expression on not a real conditional");
		}

		condition = parse_binary(node.children.at(0), scope);
		true_exp = parse_expression(node.children.at(2), scope);
		false_exp = parse_conditional(node.children.at(4), scope);
	}

	Function_Call_Expression::Function_Call_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Expression(scope) {
		node.check_type(TokenType::function_call);

		name = node.get_child_with_type(TokenType::identifier).token.value;

		// Any optional args?
		if (node.contains_child_with_type(TokenType::argument_expression_list)) {
			const auto& arg_list = node.get_child_with_type(TokenType::argument_expression_list);
			for (const auto& arg : arg_list.children) {
				if (arg.token.token_type == TokenType::comma) {
					continue; // commas are expected between arguments
				}
				arguments.push_back(parse_expression(arg, scope));
			}
		}

		// Check that the function exists and has the correct arguments.
		// Throws if there is any issue.
		scope->env->check_function(name, arguments);
	}
}
