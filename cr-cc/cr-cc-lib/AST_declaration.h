#pragma once

#include "type.h"
#include "parser.h"
#include <memory>
#include <vector>
#include <string>


namespace AST {
	class Expression;
	class VarMap;

	struct Variable {
		std::shared_ptr<Type> type;
		std::string identifier;
	};

	struct Declaration {
		std::shared_ptr<Variable> variable;
		std::shared_ptr<Expression> initialiser;
	};

	std::vector<Declaration> parse_declaration(const ParseNode& node, std::shared_ptr<VarMap> scope);
	Declaration parse_parameter_declaration(const ParseNode& node, std::shared_ptr<VarMap> scope);

	Declaration_Specifier parse_declaration_specifier(const ParseNode& node);
}
