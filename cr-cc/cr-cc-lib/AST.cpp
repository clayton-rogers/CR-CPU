#include "AST.h"

#include <stdexcept>

namespace AST {

	const Type* parse_type(const ParseNode& node, std::shared_ptr<VarMap> scope) {
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
			: env(env), scope(std::make_shared<VarMap>(env)) {

		node.check_type(TokenType::function_definition);

		return_type = parse_type(node.get_child_with_type(TokenType::type_specifier), scope);

		name = node.get_child_with_type(TokenType::identifier).token.value;

		// TODO arguments
		// arguments = ....

		contents = std::make_shared<Compount_Statement>(node.get_child_with_type(TokenType::compound_statement), scope);
	}

	VarMap::VarMap(Environment* env)
		: env(env) {
		// Add a single highest level scope
		Scope first_scope;
		first_scope.parent = NULL_SCOPE;
		scopes.push_back(first_scope);
		current_scope = 0;
	}

	VarMap::Scope_Id VarMap::create_scope() {
		Scope new_scope;
		new_scope.parent = current_scope;
		scopes.push_back(new_scope);

		Scope_Id id = scopes.size() - 1;
		set_current_scope(id);
		
		return id;
	}

	void VarMap::close_scope() {
		current_scope = scopes.at(current_scope).parent;
	}

	void VarMap::set_current_scope(Scope_Id scope) {
		current_scope = scope;
	}

	void VarMap::create_stack_var(const Type* type, const std::string& name) {
		int var_offset = size_of_scope;
		size_of_scope += type->get_size();

		// Double check that the var doesn't exist already
		if (scopes.at(current_scope).offset_map.count(name) == 1) {
			throw std::logic_error("Duplicate var with name: " + name);
		}

		Var v;
		v.is_declared = false;
		v.offset = var_offset;
		scopes.at(current_scope).offset_map[name] = v;
	}

	void VarMap::declare_var(const std::string& name) {
		if (scopes.at(current_scope).offset_map.count(name) == 1 &&
				scopes.at(current_scope).offset_map.at(name).is_declared == false) {
			scopes.at(current_scope).offset_map.at(name).is_declared = true;
		} else {
			throw std::logic_error("Should never get here: tried to declare a var that doesn't exist: " + name);
		}
	}

	int VarMap::get_var_offset(std::string name, Scope_Id* found_id) {
		Scope_Id id = current_scope;
		while (id != NULL_SCOPE) {
			// On catch vars that exist at the current or parents scopes
			// and which have been declared
			if (scopes.at(id).offset_map.count(name) == 1 &&
				scopes.at(id).offset_map.at(name).is_declared == true) {
				if (found_id != nullptr) {
					*found_id = id;
				}
				return scopes.at(id).offset_map.at(name).offset + stack_offset;
			}
			id = scopes.at(id).parent;
		}

		throw std::logic_error("Referenced unknown variable: " + name);
	}
}
