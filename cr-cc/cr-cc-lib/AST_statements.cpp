#include "AST.h"

namespace AST {

	static void add_declarations_to_stack(
			const std::vector<Declaration>& declaration_list,
			std::vector<std::shared_ptr<Statement>>* out_statement_list,
			std::shared_ptr<VarMap> scope) {

		for (const auto& declaration : declaration_list) {
			// Add var to the current scope
			out_statement_list->push_back(std::make_shared<Declaration_Statement>(declaration.variable->identifier, scope));
			scope->create_stack_var(declaration);

			// If the var has an initializer, create a fake expression statement for it
			if (declaration.initialiser) {
				auto assign_exp =
					std::make_shared<Assignment_Expression>(declaration.variable->identifier, declaration.initialiser, scope);
				out_statement_list->push_back(std::make_shared<Expression_Statement>(assign_exp, scope));
			}
		}
	}

	std::shared_ptr<Statement> parse_statement(const ParseNode& node, std::shared_ptr<VarMap> scope) {
		node.check_type(TokenType::statement);

		const auto& child = node.children.at(0);

		// statements will only have a single child
		switch (child.token.token_type) {
		case TokenType::if_statement:
			return std::make_shared<If_Statement>(child, scope);
		case TokenType::while_statement:
			return std::make_shared<While_Statement>(child, scope);
		case TokenType::do_while_statement:
			return std::make_shared<Do_While_Statement>(child, scope);
		case TokenType::for_statement:
			return std::make_shared<For_Statement>(child, scope);
		case TokenType::compound_statement:
			return std::make_shared<Compount_Statement>(child, scope);
		case TokenType::break_statement:
			return std::make_shared<Break_Statement>(child, scope);
		case TokenType::continue_statement:
			return std::make_shared<Continue_Statement>(child, scope);
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

	Return_Statement::Return_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
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

	Expression_Statement::Expression_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Statement(scope) {
		node.check_type(TokenType::expression_statement);

		// Expression statement is an expression followed by a semi colon
		if (node.contains_child_with_type(TokenType::expression)) {
			maybe_sub = parse_expression(node.children.at(0), scope);
		}
	}

	Compount_Statement::Compount_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Statement(scope) {
		node.check_type(TokenType::compound_statement);

		scope_id = scope->create_scope();

		// Compound statements can optionally have no statements inside, in which
		// case we don't do anything
		if (!node.contains_child_with_type(TokenType::block_item_list)) {
			return;
		}

		const auto& block_item_list = node.get_child_with_type(TokenType::block_item_list);

		for (const auto& block_item : block_item_list.children) {

			switch (block_item.token.token_type)
			{
				// Code blocks can optionally have a list of declarations
				case TokenType::declaration:
				{
					auto declarations = parse_declaration(block_item, scope);
					add_declarations_to_stack(declarations, &statement_list, scope);
				}
				break;
				// Code blocks can optionally have a list of statements
				case TokenType::statement:
				{
					std::shared_ptr<Statement> s = parse_statement(block_item, scope);
					this->statement_list.push_back(s);
				}
				break;
			default:
				throw std::logic_error("Compound statement should never get here: " + tokenType_to_string(block_item.token.token_type));
			}
		}

		scope->close_scope();
	}

	If_Statement::If_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Statement(scope) {
		node.check_type(TokenType::if_statement);

		condition = parse_expression(node.get_child_with_type(TokenType::expression), scope);

		// Note: in case there are two, this will get the first
		true_statement = parse_statement(node.get_child_with_type(TokenType::statement), scope);

		if (node.contains_child_with_type(TokenType::key_else)) {
			has_else = true;
			false_statement = parse_statement(node.children.at(6), scope);
		} else {
			has_else = false;
		}
	}

	Declaration_Statement::Declaration_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Statement(scope) {
		node.check_type(TokenType::identifier);
		var_name = node.token.value;
	}

	While_Statement::While_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Statement(scope) {
		node.check_type(TokenType::while_statement);

		condition = parse_expression(node.get_child_with_type(TokenType::expression), scope);
		contents = parse_statement(node.get_child_with_type(TokenType::statement), scope);
	}

	Do_While_Statement::Do_While_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Statement(scope) {
		node.check_type(TokenType::do_while_statement);

		condition = parse_expression(node.get_child_with_type(TokenType::expression), scope);
		contents = parse_statement(node.get_child_with_type(TokenType::statement), scope);
	}

	For_Statement::For_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Statement(scope) {
		node.check_type(TokenType::for_statement);

		// For statement creates its own scope to enclose any 'i' loop vars
		scope_id = scope->create_scope();

		// init expression/declaration
		const ParseNode& init_expression_node = node.children.at(2);
		if (init_expression_node.contains_child_with_type(TokenType::declaration)) {
			auto declarations = parse_declaration(init_expression_node.children.at(0), scope);
			add_declarations_to_stack(declarations, &maybe_set_up_statements, scope);
		} else if (init_expression_node.contains_child_with_type(TokenType::expression)) {
			// optional init expression exists
			const auto optional_expression = parse_expression(init_expression_node.children.at(0), scope);
			const auto expression_statement = std::make_shared<Expression_Statement>(optional_expression, scope);
			maybe_set_up_statements.push_back(expression_statement);
		} else {
			// optional init expression does not exist
		}

		// condition expression
		const ParseNode& conditional_expression_node = node.children.at(3);
		if (conditional_expression_node.contains_child_with_type(TokenType::expression)) {
			maybe_condition_statement = parse_expression(conditional_expression_node.children.at(0), scope);
		}

		// increment expression
		const ParseNode& increment_expression_node = node.children.at(4);
		if (increment_expression_node.contains_child_with_type(TokenType::expression)) {
			maybe_end_of_loop_expression = parse_expression(increment_expression_node.children.at(0), scope);
		}

		const ParseNode& contents_node = node.children.at(5);
		contents = parse_statement(contents_node, scope);

		scope->close_scope();
	}

	Break_Statement::Break_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Statement(scope) {
		node.check_type(TokenType::break_statement);
	}

	Continue_Statement::Continue_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope)
		: Statement(scope) {
		node.check_type(TokenType::continue_statement);
	}
}
