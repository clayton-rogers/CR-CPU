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
		for (const auto& external_declaration_node : root.children) {
			const auto& node = external_declaration_node.children.at(0);
			switch (node.token.token_type) {
			case TokenType::function:
			{
				// This may be a declaration or a definition.
				// In case this is a definition that has not been declared, and the function
				// is recursive, we need to always first parse it as a declaration and add it
				// to the function map, so that when the definition is parsed, it will be able
				// to find the declaration.
				auto fn = std::make_shared<Function>(node, &env, Function::Parse_Type::DECLARATION);
				env.add_function(fn);

				// If this really is only a declaration, this will just parse it as such again
				// Otherwise this will parse the definition.
				fn = std::make_shared<Function>(node, &env, Function::Parse_Type::DEFINITION);
				env.add_function(fn);

				break;
			}
			case TokenType::declaration:
			{
				auto type = parse_type(node.get_child_with_type(TokenType::type_specifier), std::make_shared<VarMap>(&env));
				auto init_decl = node
					.get_child_with_type(TokenType::init_declarator_list)
					.get_child_with_type(TokenType::init_declarator);
				auto name = init_decl.get_child_with_type(TokenType::identifier).token.value;
				std::uint16_t value = 0;
				bool has_value = false;
				if (init_decl.contains_child_with_type(TokenType::equals)) {
					// The init expression must be constant, nothing else is acceptable
					try {
						// TODO got to be a better way to do this....
						auto value_str = init_decl
							.get_child_with_type(TokenType::expression)
							.get_child_with_type(TokenType::conditional_exp)
							.get_child_with_type(TokenType::logical_or_exp)
							.get_child_with_type(TokenType::logical_and_exp)
							.get_child_with_type(TokenType::equality_exp)
							.get_child_with_type(TokenType::relational_exp)
							.get_child_with_type(TokenType::additive_exp)
							.get_child_with_type(TokenType::term)
							.get_child_with_type(TokenType::factor)
							.get_child_with_type(TokenType::constant).token.value;
						int value_int = std::stoi(value_str);
						if (value_int > 0xFFFF || value_int < -0x7FFF) {
							throw std::logic_error("Contstant literal out of range: " + value_int);
						}
						value = static_cast<std::uint16_t>(value_int);
						has_value = true;
					} catch (const std::logic_error& /*e*/) {
						throw std::logic_error("global declared with a non-constant expression");
					}
				}
				env.create_static_var(type, name, has_value, value);

				break;
			}
			default:
				throw std::logic_error("AST::AST() unexpected TokenType: " + tokenType_to_string(node.token.token_type));
			}
		}
	}

	Function::Function(const ParseNode& node, Environment* env, Parse_Type type)
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

		// If the caller is asking for only a declaration, then override
		if (is_fn_defined && type == Parse_Type::DECLARATION) {
			is_fn_defined = false;
		}

		return_type = parse_type(actual_node.get_child_with_type(TokenType::type_specifier), scope);

		name = actual_node.get_child_with_type(TokenType::identifier).token.value;

		// Function arguments
		if (actual_node.contains_child_with_type(TokenType::parameter_list)) {
			int unnamed_var_count = 0;
			const auto& parameter_list = actual_node.get_child_with_type(TokenType::parameter_list);
			for (const auto& parameter_declaration : parameter_list.children) {
				if (parameter_declaration.token.token_type == TokenType::comma) {
					continue; // comma's are expected in parameter lists
				}
				Arg_Type arg;
				arg.type = parse_type(parameter_declaration.get_child_with_type(TokenType::type_specifier), scope);
				// A function declaration parameter may not have a declarator. Ex:
				// int foo(int, int)
				if (parameter_declaration.contains_child_with_type(TokenType::init_declarator)) {
					const auto& init_declarator = parameter_declaration.get_child_with_type(TokenType::init_declarator);
					arg.name = init_declarator.get_child_with_type(TokenType::identifier).token.value;
					if (init_declarator.contains_child_with_type(TokenType::equals)) {
						arg.maybe_init_value = parse_expression(init_declarator.get_child_with_type(TokenType::expression), scope);
					}
				} else {
					arg.name = std::string("__unamed_var_") + std::to_string(unnamed_var_count++);
				}

				arguments.push_back(arg);
			}
		}

		// If this is only a declaration then we don't need to setup the scope or parse the contents
		if (is_fn_defined) {
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

			contents = std::make_shared<Compount_Statement>(actual_node.get_child_with_type(TokenType::compound_statement), scope);
		}
	}

	bool Function::signature_matches(const Function& other) const {
		if (name != other.name) return false;
		if (arguments.size() != other.arguments.size()) return false;

		for (size_t i = 0; i < arguments.size(); ++i) {
			if (arguments.at(i).type != other.arguments.at(i).type) return false;
		}

		return true;
	}

	bool Function::signature_matches(const std::string& other_name, std::vector<std::shared_ptr<Expression>> other_args) const {
		if (name != other_name) return false;
		if (arguments.size() != other_args.size()) return false;

		// TODO should also check expression type when that exists
		//for (size_t i = 0; i < arguments.size(); ++i) {
		//	if (arguments.at(i).type != other.arguments.at(i).type) return false;
		//}

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

	void Environment::add_function(std::shared_ptr<Function> new_function)	{

		auto existing_symbol = get_symbol_with_name(new_function->get_name());
		const bool symbol_exists = existing_symbol != nullptr;

		// If this is a new function, just add it to the map
		if (!symbol_exists) {
			Global_Symbol s;
			s.name = new_function->get_name();
			s.type = Global_Symbol::Type::FUNCTION;
			s.function = new_function;
			global_symbol_table.push_back(s);
			return;
		}

		// Check that we don't already have a global var by the same name
		if (symbol_exists && existing_symbol->type == Global_Symbol::Type::VARIABLE) {
			throw std::logic_error("Redeclaration of symbol: " + existing_symbol->name);
		}

		// If we already know about this function, then check that the new
		// declaration/definition is acceptable
		auto existing_fn = existing_symbol->function;
		if (!existing_fn->signature_matches(*new_function)) {
			throw std::logic_error("Tried to declare a function with incompatible signature: " + existing_fn->get_name());
		}
		if (existing_fn->is_defined() && new_function->is_defined()) {
			throw std::logic_error("Tired to define a function that is already defined: " + existing_fn->get_name());
		}

		// If this new function now contains a definition, replace our copy of it.
		if (!existing_fn->is_defined() && new_function->is_defined()) {
			existing_symbol->function = new_function;
		}
	}

	void Environment::create_static_var(const Type* type, const std::string& name, bool value_provided, std::uint16_t value) {

		// Check this symbol has not been previously declared
		auto existing_symbol = get_symbol_with_name(name);
		const bool var_already_exists = existing_symbol != nullptr;
		
		// global vars may not have the same name as functions
		if (var_already_exists && existing_symbol->type == Global_Symbol::Type::FUNCTION) {
			throw std::logic_error("Tried to declare a global var with same name as function");
		}

		if (var_already_exists) {
			const bool var_has_value = existing_symbol->static_var.has_non_default_value;
			if (!var_has_value && value_provided) {
				existing_symbol->static_var.has_non_default_value = true;
				existing_symbol->static_var.value = value;
			}
			if (var_has_value && value_provided) {
				throw std::logic_error("Tried to create a duplicate static var with name: " + name);
			}
		} else {
			// If var doesn't exist then just add it
			Global_Symbol s;
			s.name = name;
			s.type = Global_Symbol::Type::VARIABLE;
			s.static_var = Static_Var{ type, name, value_provided, value };
			global_symbol_table.push_back(s);
		}
	}

	const Type* Environment::get_static_var(const std::string& name) {

		auto symbol = get_declared_symbol_with_name(name);
		if (symbol != nullptr && symbol->type == Global_Symbol::Type::VARIABLE) {
			return symbol->static_var.type;
		}

		return nullptr;
	}

	Environment::Global_Symbol* Environment::get_symbol_with_name(const std::string& name) {
		for (auto& symbol : global_symbol_table) {
			if (symbol.name == name) {
				return &symbol;
			}
		}

		return nullptr;
	}

	Environment::Global_Symbol* Environment::get_declared_symbol_with_name(const std::string& name) {
		for (auto& symbol : global_symbol_table) {
			if (!symbol.is_declared) {
				break;
			}
			if (symbol.name == name) {
				return &symbol;
			}
		}

		return nullptr;
	}
}
