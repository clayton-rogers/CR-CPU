#include "AST.h"

namespace AST
{

	static std::shared_ptr<Expression> parse_factor(const ParseNode& node, std::shared_ptr<VarMap> scope)
	{
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
				throw std::logic_error("parse_factor(): should never get here: identifier");
				//return std::make_shared<Variable_Expression>(child, scope);
			case TokenType::function_call:
				return std::make_shared<Function_Call_Expression>(child, scope);
			default:
				throw std::logic_error("Tried to parse_factor with invalid expression type: " +
					tokenType_to_string(child.token.token_type));
		}
	}

	static std::shared_ptr<Expression> parse_binary(const ParseNode& node, std::shared_ptr<VarMap> scope)
	{
		switch (node.token.token_type) {
			// If this is one of the binary sub expressions we're good
			case TokenType::term:
			case TokenType::additive_exp:
			case TokenType::relational_exp:
			case TokenType::equality_exp:
			case TokenType::logical_and_exp:
			case TokenType::logical_or_exp:
			case TokenType::expression:
			case TokenType::shift_exp:
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

	std::shared_ptr<Expression> parse_conditional(const ParseNode& node, std::shared_ptr<VarMap> scope)
	{
		node.check_type(TokenType::conditional_exp);

		if (node.contains_child_with_type(TokenType::question)) {
			return std::make_shared<Conditional_Expression>(node, scope);
		} else {
			// Just passing through, no actual conditional
			return parse_binary(node.children.at(0), scope);
		}
	}

	std::shared_ptr<Expression> parse_expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
	{
		node.check_type(TokenType::expression);

		if (node.contains_child_with_type(TokenType::equals)) {
			// this is an assigment
			return std::make_shared<Assignment_Expression>(node, scope);
		} else {
			return parse_conditional(node.get_child_with_type(TokenType::conditional_exp), scope);
		}
	}

	Assignment_Expression::Assignment_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Expression(scope)
	{
		node.check_type(TokenType::expression);

		auto unary_exp = node.get_child_with_type(TokenType::unary_expression);
		if (unary_exp.children.size() == 1) {
			var_name = unary_exp.get_child_with_type(TokenType::identifier).token.value;
			exp = parse_expression(node.get_child_with_type(TokenType::expression), scope);
		} else if (unary_exp.contains_child_with_type(TokenType::star)) {
			var_name = unary_exp.get_child_with_type(TokenType::identifier).token.value;
			exp = parse_expression(node.get_child_with_type(TokenType::expression), scope);
			is_pointer = true;
		} else if (unary_exp.contains_child_with_type(TokenType::open_square_bracket)) {
			var_name = unary_exp.get_child_with_type(TokenType::identifier).token.value;
			exp = parse_expression(node.get_child_with_type(TokenType::expression), scope);
			array_index_exp = parse_expression(unary_exp.get_child_with_type(TokenType::expression), scope);
			is_array = true;
		} else {
			throw std::logic_error("Unknown unary expression on left side of assignment");
		}
	}

	Unary_Expression::Unary_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Expression(scope)
	{
		node.check_type(TokenType::unary_expression);

		// unary expression is always of the form <operator> <expression>
		// except in the case of an identifier.
		const auto& the_operator = node.children.at(0);
		const auto& sub_expression =
			(node.children.size() > 1) ?
			node.children.at(1) :
			ParseNode(); // unused null node

		switch (the_operator.token.token_type) {
			case TokenType::sub:
				type = Unary_Type::negation;
				break;
			case TokenType::tilda:
				type = Unary_Type::bitwise_complement;
				break;
			case TokenType::exclam:
				type = Unary_Type::logical_negation;
				break;
			case TokenType::ampersand:
				type = Unary_Type::reference;
				if (sub_expression.token.token_type != TokenType::identifier) {
					throw std::logic_error("Expected identifier when taking address of");
				}
				var_name = sub_expression.token.value;
				break;
			case TokenType::star:
				if (sub_expression.token.token_type == TokenType::identifier) {
					type = Unary_Type::dereference_identifier;
					var_name = sub_expression.token.value;
				} else {
					type = Unary_Type::dereference_expression;
				}
				break;
			case TokenType::inc_op:
				type = Unary_Type::increment;
				var_name = sub_expression.token.value;
				break;
			case TokenType::dec_op:
				type = Unary_Type::decrement;
				var_name = sub_expression.token.value;
				break;
			case TokenType::identifier:
				if (sub_expression.token.token_type == TokenType::open_square_bracket) {
					type = Unary_Type::array;
					var_name = the_operator.token.value;
					sub = parse_expression(node.get_child_with_type(TokenType::expression), scope);
				} else {
					type = Unary_Type::direct_identifier;
					var_name = the_operator.token.value;
				}
				break;
			default:
				throw std::logic_error("Invalid unary token: " + tokenType_to_string(the_operator.token.token_type));
		}

		// Do this after the operator parse, since it could generate exceptions
		if (sub_expression.token.token_type == TokenType::factor) {
			sub = parse_factor(sub_expression, scope);
		}
	}

	Constant_Expression::Constant_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Expression(scope)
	{
		node.check_type(TokenType::constant);
		size_t offset = 0;
		int value = constant_from_string(node.token.value, &offset);
		if (value > 0xFFFF || value < -0x7FFF) {
			throw std::logic_error("Contstant literal out of range: " + node.token.value);
		}
		constant_value = static_cast<std::uint16_t>(value);
	}

	std::shared_ptr<Type> Constant_Expression::get_type() const
	{
		// Constant expressions are always of type int

		return INT_TYPE;
		//Declaration_Specifier declaration_specifier;
		//declaration_specifier.specifier_list.push_back(Specifiers::INT);
		//Abstract_Declarator abstract_declarator;
		//auto type = std::make_shared<Type>(declaration_specifier, abstract_declarator);
	}

	Binary_Expression::Bin_Type Binary_Expression::token_to_type(TokenType type)
	{
		switch (type) {
			case TokenType::add:
				return Bin_Type::addition;
			case TokenType::sub:
				return Bin_Type::subtraction;
			case TokenType::star:
				return Bin_Type::multiplication;
			case TokenType::div:
				return Bin_Type::division;
			case TokenType::percent:
				return Bin_Type::remainder;
			case TokenType::and_op:
				return Bin_Type::logical_and;
			case TokenType::or_op:
				return Bin_Type::logical_or;
			case TokenType::eq_op:
				return Bin_Type::equal;
			case TokenType::ne_op:
				return Bin_Type::not_equal;
			case TokenType::less_than:
				return Bin_Type::less_than;
			case TokenType::le_op:
				return Bin_Type::less_than_or_equal;
			case TokenType::greater_than:
				return Bin_Type::greater_than;
			case TokenType::ge_op:
				return Bin_Type::greater_than_or_equal;
			case TokenType::left_op:
				return Bin_Type::shift_left;
			case TokenType::right_op:
				return Bin_Type::shift_right;
			default:
				throw std::logic_error("Tried to convert invalid TokenType to binary expression: "
					+ tokenType_to_string(type));
		}
	}

	std::shared_ptr<Type> Binary_Expression::get_type() const
	{
		// TODO need to determine combined expression from two sub expressions

		const auto left_broad_type = sub_left->get_type()->get_broad_type();
		const auto right_broad_type = sub_right->get_type()->get_broad_type();

		// For now just return int, it basically will be always anyway
		if (left_broad_type == Broad_Type::INTEGRAL &&
			right_broad_type == Broad_Type::INTEGRAL) {
			// if both are int, then return int
			// TODO should be the largest of the two or something
			return INT_TYPE;
			//Declaration_Specifier declaration_specifiers;
			//declaration_specifiers.specifier_list.push_back(Specifiers::INT);
			//return std::make_shared<Type>(declaration_specifiers, Abstract_Declarator());
		} else if (left_broad_type == Broad_Type::POINTER) {
			return sub_left->get_type();
		} else if (right_broad_type == Broad_Type::POINTER) {
			return sub_right->get_type();
		} else if (left_broad_type == Broad_Type::STRUCT ||
			right_broad_type == Broad_Type::STRUCT) {
			// There are no valid binary operators on strucs
			throw std::logic_error("Invalid binary operator on struct");
		} else if (left_broad_type == Broad_Type::ARRAY ||
			right_broad_type == Broad_Type::ARRAY) {
			// There are no valid binary operators on arrays either
			throw std::logic_error("Invalid binary operator on array");
		}

		// If we got this far then we probably forgot something
		throw std::logic_error("Invalid binary operator on unknown sub-expression types");
	}

	// std::shared_ptr<Type> Variable_Expression::get_type() const
	// {
	// 	// TODO should be able to query the scope for the type of a given var

	// 	// Default to int for now
	// 	return INT_TYPE;

	// 	//Declaration_Specifier declaration_specifier;
	// 	//declaration_specifier.specifier_list.push_back(Specifiers::INT);
	// 	//Abstract_Declarator abstract_declarator;
	// 	//auto type = std::make_shared<Type>(declaration_specifier, abstract_declarator);
	// }

	Conditional_Expression::Conditional_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Expression(scope)
	{
		node.check_type(TokenType::conditional_exp);
		if (!node.contains_child_with_type(TokenType::question)) {
			throw std::logic_error("Called conditional_expression on not a real conditional");
		}

		condition = parse_binary(node.children.at(0), scope);
		true_exp = parse_expression(node.children.at(2), scope);
		false_exp = parse_conditional(node.children.at(4), scope);
	}

	std::shared_ptr<Type> Conditional_Expression::get_type() const
	{
		// In theory both true and false sides of the expresion should be the same time right????
		auto true_type = true_exp->get_type();
		auto false_type = false_exp->get_type();
		if (true_type->is_same(false_type)) {
			return true_type;
		} else {
			throw std::logic_error("Type mismatch in conditional expression (?:)");
		}
	}

	Function_Call_Expression::Function_Call_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Expression(scope)
	{
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

	std::shared_ptr<Type> Function_Call_Expression::get_type() const
	{
		// The type of a function call will of course be whatever the return type of the function is
		// We should be able to query the function from the symbol table

		return scope->env->get_function_return_type(name);
	}
}
