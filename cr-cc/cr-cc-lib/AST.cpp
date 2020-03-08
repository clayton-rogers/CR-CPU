#include "AST.h"

#include <stdexcept>

namespace AST {

	const Type* parse_type(const ParseNode& node, std::shared_ptr<Scope> scope) {
		node.check_type(TokenType::type_specifier);

		const auto& child = node.children.at(0);

		switch (child.token.token_type) {
		case TokenType::key_int:
			return scope->env->get_type("int");
		default:
			throw std::logic_error("Tried to parse_type with invalid type: " +
				tokenType_to_string(child.token.token_type));
		}
	}

	// Class functions
	class Int_Type : public Type {
	public:
		virtual int get_size() const { return 1; }
		virtual std::string get_name() const { return "int"; }
		virtual bool is_complete() const { return true; }
	};

	AST::AST(const ParseNode& root) {
		root.check_type(TokenType::translation_unit);

		// Set up default types
		Type* my_int = new Int_Type();
		env.create_type(my_int);

		// Parse each function and type definition
		for (const auto& node : root.children) {
			if (node.token.token_type == TokenType::function_definition) {
				auto fn = std::make_shared<Function>(node, &env);
				functions.push_back(fn);
			}
		}
	}

	Function::Function(const ParseNode& node, Environment* env)
			: env(env), scope(std::make_shared<Scope>(env)) {

		node.check_type(TokenType::function_definition);

		return_type = parse_type(node.get_child_with_type(TokenType::type_specifier), scope);

		name = node.get_child_with_type(TokenType::identifier).token.value;

		// TODO arguments
		// arguments = ....

		contents = std::make_shared<Compount_Statement>(node.get_child_with_type(TokenType::compound_statement), scope);
	}
}
