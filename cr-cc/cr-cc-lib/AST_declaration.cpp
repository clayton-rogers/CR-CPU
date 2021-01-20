#include "AST_declaration.h"

#include "AST.h"

#include <stdexcept>
#include <map>
#include <string>

namespace AST {

	static Declaration parse_init_declarator(const Declaration_Specifier& ds, const ParseNode& node, std::shared_ptr<VarMap> scope) {
		node.check_type(TokenType::init_declarator);

		const auto& declarator = node.get_child_with_type(TokenType::declarator);
		const auto& direct_declarator = declarator.get_child_with_type(TokenType::direct_declarator);

		const auto& identifier = direct_declarator.get_child_with_type(TokenType::identifier).token.value;

		int num_pointer = 0;
		if (declarator.contains_child_with_type(TokenType::pointer)) {
			const auto& ptr_node = declarator.get_child_with_type(TokenType::pointer);
			num_pointer = static_cast<int>(ptr_node.children.size());
		}

		std::vector<int> array_sizes;
		if (direct_declarator.contains_child_with_type(TokenType::open_square_bracket)) {
			// Then this is an array declaration
			// TODO assume for now that there is only ever one dimension
			int array_size = std::stoi(direct_declarator.get_child_with_type(TokenType::constant).token.value);
			if (array_size > 0xFFFF) {
				throw std::logic_error("array size out of range");
			}
			array_sizes.push_back(array_size);
		}

		Abstract_Declarator abstract_declarator;
		abstract_declarator.num_pointers = num_pointer;
		abstract_declarator.array_sizes = array_sizes;
		// TODO be able to parse nested declarators and arrays

		auto variable = std::make_shared<Variable>();
		variable->identifier = identifier;
		variable->type = std::make_shared<Type>(ds, abstract_declarator);

		Declaration declaration;
		declaration.variable = variable;
		if (node.contains_child_with_type(TokenType::expression)) {
			declaration.initialiser = parse_expression(node.get_child_with_type(TokenType::expression), scope);
		}

		return declaration;
	}

	std::vector<Declaration> parse_declaration(const ParseNode& node, std::shared_ptr<VarMap> scope) {
		node.check_type(TokenType::declaration);

		std::vector<Declaration> declarations;

		Declaration_Specifier declaration_specifier = parse_declaration_specifier(node.get_child_with_type(TokenType::declaration_specifier));
		const auto& init_declarator_list = node.get_child_with_type(TokenType::init_declarator_list);
		for (const auto& init_declarator : init_declarator_list.children) {
			declarations.push_back(parse_init_declarator(declaration_specifier, init_declarator, scope));
		}

		return declarations;
	}

	Declaration parse_parameter_declaration(const ParseNode& node, std::shared_ptr<VarMap> scope) {
		node.check_type(TokenType::parameter_declaration);


		Declaration_Specifier declaration_specifier = parse_declaration_specifier(node.get_child_with_type(TokenType::declaration_specifier));
		if (node.contains_child_with_type(TokenType::init_declarator)) {
			auto declaration = parse_init_declarator(declaration_specifier, node.get_child_with_type(TokenType::init_declarator), scope);
			return declaration;
		}

		// If there's no declarator then create an empty one
		// this can happen in forward declarations
		// ex. int foo(int);
		Abstract_Declarator abstract_declarator;
		auto variable = std::make_shared<Variable>();
		variable->type = std::make_shared<Type>(declaration_specifier, abstract_declarator);
		variable->identifier = ""; // no effect, but deliberately empty
		Declaration declaration;
		declaration.variable = variable;

		return declaration;
	}

	Declaration_Specifier parse_declaration_specifier(const ParseNode& node) {
		node.check_type(TokenType::declaration_specifier);

		Declaration_Specifier ds;

		const std::map<TokenType, Specifiers> specifier_map = {
			{TokenType::key_typedef, Specifiers::TYPEDEF},
			{TokenType::key_extern, Specifiers::EXTERN},
			{TokenType::key_static, Specifiers::STATIC},
			{TokenType::key_auto, Specifiers::AUTO},

			{TokenType::key_void, Specifiers::VOID},
			{TokenType::key_int, Specifiers::INT},
			{TokenType::key_struct, Specifiers::STRUCT},
			{TokenType::key_enum, Specifiers::ENUM},
			{TokenType::identifier, Specifiers::TYPEDEF_NAME},

			{TokenType::key_const, Specifiers::CONST},
			{TokenType::key_volatile, Specifiers::VOLATILE},
		};

		for (const auto& child : node.children) {
			const auto token_type = child.token.token_type;
			if (specifier_map.count(token_type) == 0) {
				throw std::logic_error("Unexpected token in declaration specifier: " + tokenType_to_string(token_type));
			}
			ds.specifier_list.push_back(specifier_map.at(token_type));
		}

		return ds;
	}

}

