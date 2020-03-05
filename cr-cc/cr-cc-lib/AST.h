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
		virtual std::string generate_code() const = 0;
	};

	class Type {
	public:
		virtual int get_size() = 0;
		virtual std::string get_name() = 0;
		virtual bool is_complete() = 0;
	};

	class Variable {
	public:
		Variable(std::string name, std::shared_ptr<Type> type) :
			type(type), name(name) {}
		std::string get_name() const { return name; }
		std::shared_ptr<Type> get_type() const { return type; }
	private:
		std::shared_ptr<Type> type;
		std::string name;
	};

	using Type_Map_Type = std::map<std::string, std::shared_ptr<Type>>;

	class Scope {
	public:
		Scope(std::shared_ptr<Type_Map_Type> type_map)
			: label_maker(std::make_shared<Label_Maker>()),
			  type_map(type_map)
			{}
		std::shared_ptr<Scope> create_child_scope();

		std::shared_ptr<Variable> get_var(std::string name) const;
		void add_var(std::shared_ptr<Variable> var);

		std::shared_ptr<Type> get_type(std::string name) const;
		void add_type(std::shared_ptr<Type> type);

		void modify_stack_offset(int amount) { stack_offset += amount; }

		std::string gen_scope_entry();
		std::string gen_scope_exit();

		std::shared_ptr<Label_Maker> label_maker;
	private:
		const Scope* parent = nullptr;
		std::map<std::string, std::shared_ptr<Variable>> symbol_table;
		std::shared_ptr<Type_Map_Type> type_map;
		int stack_offset = 0;
	};
	std::shared_ptr<Type> parse_type(const ParseNode& node, std::shared_ptr<Scope> scope);

	class Expression : public Compilable {
	public:
		Expression(std::shared_ptr<Scope> scope) : scope(scope) {}
		virtual ~Expression() {}
	protected:
		std::shared_ptr<Scope> scope;
	};
	std::shared_ptr<Expression> parse_expression(const ParseNode& node, std::shared_ptr<Scope> scope);

	class Unary_Expression : public Expression {
	public:
		enum class Type {
			negation,
			bitwise_complement,
			logical_negation,
		};
		Unary_Expression(const ParseNode& node, std::shared_ptr<Scope> scope);
		~Unary_Expression() {}
		std::string generate_code() const override;
	private:
		Type type;
		std::shared_ptr<Expression> sub;
	};

	class Constant_Expression : public Expression {
	public:
		Constant_Expression(const ParseNode& node, std::shared_ptr<Scope> scope);
		~Constant_Expression() {}
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
		~Binary_Expression() {}
		std::string generate_code() const override;
	private:
		static Type token_to_type(TokenType type);
		Type type;
		std::shared_ptr<Expression> sub_left;
		std::shared_ptr<Expression> sub_right;
	};

	class Statement : public Compilable {
	public:
		Statement(std::shared_ptr<Scope> scope) : scope(scope) {}
		//virtual std::string generate_code() const override { return ""; }; // TODO
		virtual ~Statement() {};
	protected:
		std::shared_ptr<Scope> scope;
	};
	std::shared_ptr<Statement> parse_statement(const ParseNode& node, std::shared_ptr<Scope> scope);

	class Return_Statement : public Statement {
	public:
		Return_Statement(const ParseNode& node, std::shared_ptr<Scope> scope);
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> ret_expression;
	};

	class Code_Block : public Compilable {
	public:
		Code_Block(const ParseNode& node, std::shared_ptr<Scope> scope);
		//virtual std::string generate_code() const override { return ""; }; // TODO
		virtual ~Code_Block() {};
		std::string generate_code() const override;
	private:
		std::shared_ptr<Scope> scope;
		std::vector<std::shared_ptr<Statement>> statement_list;
	};

	class Function :public Compilable {
	public:
		Function(const ParseNode& node, std::shared_ptr<Scope> scpoe);
		virtual ~Function() {};
		std::string generate_code() const;
	private:
		std::shared_ptr<Scope> scope;
		std::shared_ptr<Type> return_type;
		std::string name;
		std::vector<std::shared_ptr<Variable>> arguments;
		std::shared_ptr<Code_Block> contents;
	};

	class AST final : public Compilable {
	public:
		AST(const ParseNode& root);
		std::string generate_code() const;
	private:
		std::shared_ptr<Scope> global_scope;
		std::vector<std::shared_ptr<Function>> functions;
	};
}
