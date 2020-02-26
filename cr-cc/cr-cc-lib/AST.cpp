#include "AST.h"

#include <stdexcept>

namespace AST {

	std::shared_ptr<Type> parse_type(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::type_specifier);

		const auto& child = node.children.at(0);

		switch (child.token.token_type) {
		case TokenType::key_int:
			return scope->get_type("int");
		default:
			throw std::logic_error("Tried to parse_type with invalid type: " +
				tokenType_to_string(child.token.token_type));
		}
	}

	// Class functions
	class Int_Type : public Type {
	public:
		virtual int get_size() { return 1; }
		virtual std::string get_name() { return "int"; }
		virtual bool is_complete() { return true; }
	};

	AST::AST(const ParseNode& root) {
		root.check_type(TokenType::translation_unit);

		// Create the global scope
		auto type_map = std::make_shared<Type_Map_Type>();
		global_scope = std::make_shared<Scope>(type_map);

		// Set up default types
		auto my_int = std::make_shared<Int_Type>();
		(*type_map)[my_int->get_name()] = my_int;

		// Parse each function and type definition
		for (const auto& node : root.children) {
			if (node.token.token_type == TokenType::function_definition) {
				auto fn = std::make_shared<Function>(node, global_scope);
				functions.push_back(fn);
			}
		}
	}

	Code_Block::Code_Block(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::compound_statement);

		this->scope = scope->create_child_scope();

		// Code blocks only optionally have a list of statments
		if (node.contains_child_with_type(TokenType::statement_list)) {
			const auto& list = node.get_child_with_type(TokenType::statement_list);
			for (const auto& statement_node : list.children) {
				std::shared_ptr<Statement> s = parse_statement(statement_node, this->scope);
				this->statement_list.push_back(s);
			}
		}
	}

	Function::Function(const ParseNode& node, std::shared_ptr<Scope> current_scope) {
		node.check_type(TokenType::function_definition);

		scope = current_scope->create_child_scope();

		return_type = parse_type(node.get_child_with_type(TokenType::type_specifier), scope);

		name = node.get_child_with_type(TokenType::identifier).token.value;

		// TODO arguments
		// arguments = ....

		contents = std::make_shared<Code_Block>(node.get_child_with_type(TokenType::compound_statement), scope);
	}

	std::shared_ptr<Scope> Scope::create_child_scope() {
		// Create a child scope with the same type_map
		auto child = std::make_shared<Scope>(type_map);
		child->parent = this;
		child->label_maker = this->label_maker; // All scopes share a set of labels
		return child;
	}

	std::shared_ptr<Variable> Scope::get_var(std::string name) const {
		if (symbol_table.count(name) == 1) {
			return symbol_table.at(name);
		} else {
			if (parent == nullptr) {
				return nullptr;
			} else {
				return parent->get_var(name);
			}
		}
	}

	void Scope::add_var(std::shared_ptr<Variable> var) {
		symbol_table[var->get_name()] = var;
	}

	std::shared_ptr<Type> Scope::get_type(std::string name) const {
		if (type_map->count(name) == 1) {
			return type_map->at(name);
		} else {
			return nullptr;
		}
	}
	void Scope::add_type(std::shared_ptr<Type> type) {
		(*type_map)[type->get_name()] = type;
	}

}
