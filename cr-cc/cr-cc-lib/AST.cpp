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
		int get_size() const override { return 1; }
		std::string get_name() const override { return "int"; }
		bool is_complete() const override { return true; }
	};

	class Void_Type : public Type {
	public:
		int get_size() const override { return 0; }
		std::string get_name() const override { return "void"; }
		bool is_complete() const override { return true; }
	};

	AST::AST(const ParseNode& root) {
		root.check_type(TokenType::translation_unit);

		// Set up default types
		Type* my_int = new Int_Type();
		env.create_type(my_int);
		Type* my_void = new Void_Type();
		env.create_type(my_void);

		// Parse each function and type definition
		for (const auto& node : root.children) {
			switch (node.token.token_type) {
			case TokenType::function:
			{
				auto fn = std::make_shared<Function>(node, &env);
				env.add_function(fn);
				break;
			}
			default:
				throw std::logic_error("AST::AST() unexpected TokenType: " + tokenType_to_string(node.token.token_type));
			}
		}
	}

	Function::Function(const ParseNode& node, Environment* env)
			: env(env), scope(std::make_shared<VarMap>(env)) {
		node.check_type(TokenType::function);

		if (node.contains_child_with_type(TokenType::function_definition)) {
			is_fn_defined = true;
		} else {
			is_fn_defined = false;
		}

		const auto& actual_node = node.children.at(0);
		if (is_fn_defined) {
			actual_node.check_type(TokenType::function_definition);
		} else {
			actual_node.check_type(TokenType::function_declaration);
		}

		return_type = parse_type(actual_node.get_child_with_type(TokenType::type_specifier), scope);

		name = actual_node.get_child_with_type(TokenType::identifier).token.value;

		// Function arguments
		if (actual_node.contains_child_with_type(TokenType::parameter_list)) {
			const auto& parameter_list = actual_node.get_child_with_type(TokenType::parameter_list);
			for (const auto& parameter_declaration : parameter_list.children) {
				if (parameter_declaration.token.token_type == TokenType::comma) {
					continue; // comma's are expected in parameter lists
				}
				Arg_Type arg;
				arg.type = parse_type(parameter_declaration.get_child_with_type(TokenType::type_specifier), scope);
				const auto& init_declarator = parameter_declaration.get_child_with_type(TokenType::init_declarator);
				arg.name = init_declarator.get_child_with_type(TokenType::identifier).token.value;
				if (init_declarator.contains_child_with_type(TokenType::equals)) {
					arg.maybe_init_value = parse_expression(init_declarator.get_child_with_type(TokenType::expression), scope);
				}

				arguments.push_back(arg);
			}
		}

		// Note this must be kept in line with Function::generate_code()
		if (arguments.size() >= 1) {
			scope->create_stack_var_at_offset(0, arguments.at(0).name);
		} else {
			scope->create_stack_var_at_offset(0, "__saved_ra");
		}
		if (arguments.size() >= 2) {
			scope->create_stack_var_at_offset(1, arguments.at(1).name);
		} else {
			scope->create_stack_var_at_offset(1, "__saved_rb");
		}
		scope->create_stack_var_at_offset(2, "__return_addr");
		for (std::size_t arg_index = 2; arg_index < arguments.size(); ++arg_index) {
			scope->create_stack_var_at_offset(static_cast<int>(arg_index) + 1, arguments.at(arg_index).name);
		}

		if (is_fn_defined) {
			contents = std::make_shared<Compount_Statement>(actual_node.get_child_with_type(TokenType::compound_statement), scope);
		}
	}

	bool Function::signature_matches(const Function& other) {
		if (name != other.name) return false;
		if (arguments.size() != other.arguments.size()) return false;

		for (size_t i = 0; i < arguments.size(); ++i) {
			if (arguments.at(i).type != other.arguments.at(i).type) return false;
		}

		return true;
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
		//int var_offset = size_of_var_map;
		size_of_var_map += type->get_size();

		// Double check that the var doesn't exist already
		if (scopes.at(current_scope).offset_map.count(name) == 1) {
			throw std::logic_error("Duplicate var with name: " + name);
		}

		Var v;
		v.is_declared = false;
		v.offset = -type->get_size(); // once all args are incremented, this will be zero
		scopes.at(current_scope).offset_map[name] = v;

		increment_all_vars(type->get_size());
	}

	void VarMap::increment_all_vars(int offset) {
		for (auto& scope : scopes) {
			for (auto& var : scope.offset_map) {
				var.second.offset += offset;
			}
		}
	}

	void VarMap::create_stack_var_at_offset(int offset, const std::string& name) {
		// Double check that the var doesn't exist already
		if (scopes.at(current_scope).offset_map.count(name) == 1) {
			throw std::logic_error("Duplicate var with name: " + name);
		}

		Var v;
		v.is_declared = true;
		v.offset = offset;
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

	void Environment::add_function(std::shared_ptr<Function> new_function)	{
		// If this is a new function, just add it to the map
		if (function_map.count(new_function->get_name()) == 0) {
			function_map[new_function->get_name()] = new_function;
			return;
		}

		// If we already know about this function, then check that the new
		// declaration/definition is acceptable
		auto existing_fn = function_map.at(new_function->get_name());
		if (!existing_fn->signature_matches(*new_function)) {
			throw std::logic_error("Tried to declare a function with incompatible signature: " + existing_fn->get_name());
		}
		if (existing_fn->is_defined() && new_function->is_defined()) {
			throw std::logic_error("Tired to define a function that is already defined: " + existing_fn->get_name());
		}

		// If this new function now contains a definition, replace our copy of it.
		if (!existing_fn->is_defined() && new_function->is_defined()) {
			function_map.at(new_function->get_name()) = new_function;
		}
	}
}
