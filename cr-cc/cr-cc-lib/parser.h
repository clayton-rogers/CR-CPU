#pragma once

#include "tokenizer.h"

#include <string>
#include <vector>

struct VariableType {
	std::string name;
	int size;
};

struct Declaration {
	bool is_const;
	VariableType type;
	std::string name;
};

enum class StatementSwitch {
	statement_list,
	actual_statement,
};

struct Statement {
	// One of:
	StatementSwitch statement_switch;
	// 1: Statement list:
	std::vector<Statement> statement_list;
	// 2: Actual statement:

};

struct Function {
	VariableType return_type;
	std::string name;
	std::vector<Declaration> arguments;
	Statement contents;
};

struct ParseTree {
	std::vector<Declaration> declarations;
	std::vector<Function> functions;
};

ParseTree parse(TokenList token_list);
