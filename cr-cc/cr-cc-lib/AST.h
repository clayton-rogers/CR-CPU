#pragma once

#include "parser.h"
#include "label_maker.h"

#include <vector>
#include <map>
#include <string>
#include <memory>

namespace AST {

	class Compilable {
	public:
		virtual ~Compilable() {}
		virtual std::string generate_code() const = 0;
	};

	class Type {
	public:
		virtual ~Type() {}
		virtual int get_size() const = 0;
		virtual std::string get_name() const = 0;
		virtual bool is_complete() const = 0;
	};

	class Environment {
	public:
		using Type_Map_Type = std::map<std::string, Type*>;
		~Environment() {
			for (const auto& t : type_map) {
				delete t.second;
			}
		}
		const Type* get_type(std::string name) const;
		void create_type(Type* type);

		Label_Maker label_maker;
		// TODO static vars
	private:
		Type_Map_Type type_map;
	};

	//class Variable {
	//public:
	//	virtual ~Variable() {}
	//	virtual std::string gen_load(int current_sp_offset) = 0;
	//	virtual std::string gen_store(int current_sp_offset) = 0;

	//	Variable(std::string name, const Type* type) :
	//		type(type), name(name) {}
	//	std::string get_name() const { return name; }
	//	const Type* get_type() const { return type; }
	//protected:
	//	const Type* type;
	//	std::string name;
	//};

	//class Local_Variable : public Variable {
	//public:
	//	Local_Variable(std::string name, const Type* type, int sp_offset)
	//		: Variable(name, type), sp_offset(sp_offset) {}

	//	std::string gen_load(int current_sp_offset) override;
	//	std::string gen_store(int current_sp_offset) override;
	//private:
	//	int sp_offset;
	//};

	class Scope {
	public:
		Scope(Environment* env)
			: env(env)
		{}

		void create_stack_var(const Type* type, std::string name);
		int get_var_offset(std::string name);
		std::string push_reg(std::string reg_name);
		std::string pop_reg(std::string reg_name);

		//int get_current_offset() { return stack_offset; }

		std::string gen_scope_entry();
		std::string gen_scope_exit();

		Environment* env;
	private:
		struct Var_Offset {
			int offset = 0;
			std::string name;
		};
		std::vector<Var_Offset> vars;
		int size_of_scope = 0;
		int stack_offset = 0;// size of temporaries
	};
	const Type* parse_type(const ParseNode& node, std::shared_ptr<Scope> scope);

	class Expression : public Compilable {
	public:
		Expression(std::shared_ptr<Scope> scope) : scope(scope) {}
		virtual ~Expression() {}
	protected:
		std::shared_ptr<Scope> scope;
	};
	std::shared_ptr<Expression> parse_expression(const ParseNode& node, std::shared_ptr<Scope> scope);

	class Assignment_Expression : public Expression {
	public:
		Assignment_Expression(const ParseNode& node, std::shared_ptr<Scope> scope);
		Assignment_Expression(std::string name, std::shared_ptr<Expression> exp, std::shared_ptr<Scope> scope)
			: Expression(scope), var_name(name), exp(exp) {}
		std::string generate_code() const override;
	private:
		std::string var_name;
		std::shared_ptr<Expression> exp;
	};

	// For when a variable is referenced
	class Variable_Expression : public Expression {
	public:
		Variable_Expression(const ParseNode& node, std::shared_ptr<Scope> scope);
		std::string generate_code() const override;
	private:
		std::string var_name;
	};

	class Unary_Expression : public Expression {
	public:
		enum class Type {
			negation,
			bitwise_complement,
			logical_negation,
		};
		Unary_Expression(const ParseNode& node, std::shared_ptr<Scope> scope);
		std::string generate_code() const override;
	private:
		Type type;
		std::shared_ptr<Expression> sub;
	};

	class Constant_Expression : public Expression {
	public:
		Constant_Expression(const ParseNode& node, std::shared_ptr<Scope> scope);
		std::string generate_code() const override;
	private:
		std::uint16_t constant_value;
	};

	class Binary_Expression : public Expression {
	public:
		enum class Type {
			addition,
			subtraction,
			multiplication,
			division,
			logical_and,
			logical_or,
			equal,
			not_equal,
			less_than,
			less_than_or_equal,
			greater_than,
			greater_than_or_equal,
		};
		Binary_Expression(
			TokenType type,
			std::shared_ptr<Expression> left,
			std::shared_ptr<Expression> right,
			std::shared_ptr<Scope> scope)
				: Expression(scope),
				  type(token_to_type(type)),
				  sub_left(left),
				  sub_right(right)
				  {}
		std::string generate_code() const override;
	private:
		static Type token_to_type(TokenType type);
		Type type;
		std::shared_ptr<Expression> sub_left;
		std::shared_ptr<Expression> sub_right;
	};

	class Conditional_Expression : public Expression {
	public:
		Conditional_Expression(const ParseNode& node, std::shared_ptr<Scope> scope);
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> condition;
		std::shared_ptr<Expression> true_exp;
		std::shared_ptr<Expression> false_exp;
	};

	class Statement : public Compilable {
	public:
		Statement(std::shared_ptr<Scope> scope) : scope(scope) {}
	protected:
		std::shared_ptr<Scope> scope;
	};
	std::shared_ptr<Statement> parse_statement(const ParseNode& node, std::shared_ptr<Scope> scope);
	// A single declaration may generate any number of statments (and any number of variables)
	std::vector<std::shared_ptr<Statement>> parse_declaration(const ParseNode& node, std::shared_ptr<Scope> scope);

	class Return_Statement : public Statement {
	public:
		Return_Statement(const ParseNode& node, std::shared_ptr<Scope> scope);
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> ret_expression;
	};

	class Expression_Statement : public Statement {
	public:
		Expression_Statement(const ParseNode& node, std::shared_ptr<Scope> scope);
		Expression_Statement(std::shared_ptr<Expression> sub, std::shared_ptr<Scope> scope)
			: Statement(scope), sub(sub) {}
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> sub;
	};

	class Compount_Statement : public Statement {
	public:
		Compount_Statement(const ParseNode& node, std::shared_ptr<Scope> scope);
		std::string generate_code() const override;
	private:
		std::vector<std::shared_ptr<Statement>> statement_list;
	};

	class If_Statement : public Statement {
	public:
		If_Statement(const ParseNode& node, std::shared_ptr<Scope> scope);
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> condition;
		bool has_else;
		std::shared_ptr<Statement> true_statement;
		std::shared_ptr<Statement> false_statement;
	};

	class Function :public Compilable {
	public:
		Function(const ParseNode& node, Environment* env);
		virtual ~Function() {};
		std::string generate_code() const;
	private:
		Environment* env;
		std::shared_ptr<Scope> scope;
		const Type* return_type;
		std::string name;
		std::vector<std::string>arguments;
		std::shared_ptr<Compount_Statement> contents;
	};

	class AST final : public Compilable {
	public:
		AST(const ParseNode& root);
		std::string generate_code() const;
	private:
		Environment env;
		//std::shared_ptr<Scope> global_scope;
		std::vector<std::shared_ptr<Function>> functions;
	};
}
