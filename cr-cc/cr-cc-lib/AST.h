#pragma once

#include "parser.h"
#include "label_maker.h"
#include "type.h"
#include "AST_declaration.h"

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

	class Environment;

	struct Static_Var {
		Declaration declaration;
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

	class VarMap {
	public:
		using Scope_Id = std::size_t; // TODO change to signed type so -1 can be special?
		static const Scope_Id MAGIC_NO_SCOPE = 10000; // Magic value indicating no scope was found
		struct Loop_Labels {
			std::string top;
			std::string after;
		};

		VarMap(Environment* env);

		Scope_Id create_scope();
		void close_scope();
		void set_current_scope(Scope_Id scope);

		void create_stack_var(const Declaration& declaration);
		void create_stack_var_at_offset(int offset, const std::string& name);
		void declare_var(const std::string& name);

		std::string store_word(const std::string& name, const std::string& source_reg);
		std::string load_word(const std::string& name, const std::string& dest_reg);
		std::string load_address(const std::string& name, const std::string& dest_reg);

		std::string push_reg(std::string reg_name);
		std::string pop_reg(std::string reg_name);

		void push_loop(const Loop_Labels& l) {
			loop_labels.push_back(l);
		}
		void pop_loop() {
			loop_labels.pop_back();
		}

		std::string get_top_label() {
			if (loop_labels.size() == 0) {
				throw std::logic_error("Break or continue not in loop");
			}
			return loop_labels.back().top;
		}
		std::string get_after_label() {
			if (loop_labels.size() == 0) {
				throw std::logic_error("Break or continue not in loop");
			}
			return loop_labels.back().after;
		}

		std::string gen_scope_entry();
		std::string gen_scope_exit();

		Environment* env;
	private:
		struct Var {
			int offset = 0;
			bool is_declared = false;
		};
		struct Scope {
			Scope_Id parent;
			std::map<std::string, Var> offset_map;
		};
		std::vector<Scope> scopes;
		Scope_Id current_scope = 0;
		static const Scope_Id NULL_SCOPE = static_cast<Scope_Id>(-1);

		std::vector<Loop_Labels> loop_labels;

		int size_of_var_map = 0;
		int stack_offset = 0;// size of temporaries

		void increment_all_vars(int offset);
		int get_var_offset(const std::string& name, Scope_Id* found_id);
	};

	class Expression : public Compilable {
	public:
		Expression(std::shared_ptr<VarMap> scope) : scope(scope) {}
		virtual ~Expression() {}
		virtual std::shared_ptr<Type> get_type() const = 0;
	protected:
		std::shared_ptr<VarMap> scope;
	};
	std::shared_ptr<Expression> parse_expression(const ParseNode& node, std::shared_ptr<VarMap> scope);

	class Assignment_Expression : public Expression {
	public:
		Assignment_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope);
		Assignment_Expression(std::string name, std::shared_ptr<Expression> exp, std::shared_ptr<VarMap> scope)
			: Expression(scope), var_name(name), exp(exp) {}
		std::string generate_code() const override;
		std::shared_ptr<Type> get_type() const override { return exp->get_type(); }
	private:
		std::string var_name;
		bool is_pointer = false;
		std::shared_ptr<Expression> exp;
		bool is_array = false;
		std::shared_ptr<Expression> array_index_exp;
	};

	// For when a variable is referenced
	class Variable_Expression : public Expression {
	public:
		enum class Variable_Type {
			direct,
			reference,
			dereference,
			array,
		};

		Variable_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
		std::shared_ptr<Type> get_type() const override;
	private:
		Variable_Type var_type;
		std::string var_name;
		std::shared_ptr<Expression> array_index;
	};

	class Unary_Expression : public Expression {
	public:
		enum class Unary_Type {
			negation,
			bitwise_complement,
			logical_negation,
		};
		Unary_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
		std::shared_ptr<Type> get_type() const override { return sub->get_type(); }
	private:
		Unary_Type type;
		std::shared_ptr<Expression> sub;
	};

	class Constant_Expression : public Expression {
	public:
		Constant_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;

		std::uint16_t get_value() const { return constant_value; }
		std::shared_ptr<Type> get_type() const override;
	private:
		std::uint16_t constant_value;
	};

	class Binary_Expression : public Expression {
	public:
		enum class Bin_Type {
			addition,
			subtraction,
			multiplication,
			division,
			remainder,
			logical_and,
			logical_or,
			equal,
			not_equal,
			less_than,
			less_than_or_equal,
			greater_than,
			greater_than_or_equal,
			shift_left,
			shift_right,
		};
		Binary_Expression(
			TokenType type,
			std::shared_ptr<Expression> left,
			std::shared_ptr<Expression> right,
			std::shared_ptr<VarMap> scope)
				: Expression(scope),
				  type(token_to_type(type)),
				  sub_left(left),
				  sub_right(right)
				  {}
		std::string generate_code() const override;
		std::shared_ptr<Type> get_type() const override;
	private:
		static Bin_Type token_to_type(TokenType type);
		Bin_Type type;
		std::shared_ptr<Expression> sub_left;
		std::shared_ptr<Expression> sub_right;
	};

	class Conditional_Expression : public Expression {
	public:
		Conditional_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
		std::shared_ptr<Type> get_type() const override;
	private:
		std::shared_ptr<Expression> condition;
		std::shared_ptr<Expression> true_exp;
		std::shared_ptr<Expression> false_exp;
	};

	class Function_Call_Expression : public Expression {
	public:
		Function_Call_Expression(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
		std::shared_ptr<Type> get_type() const override;
	private:
		std::string name;
		std::vector<std::shared_ptr<Expression>> arguments;
	};

	class Statement : public Compilable {
	public:
		Statement(std::shared_ptr<VarMap> scope) : scope(scope) {}
	protected:
		std::shared_ptr<VarMap> scope;
	};
	std::shared_ptr<Statement> parse_statement(const ParseNode& node, std::shared_ptr<VarMap> scope);

	class Return_Statement : public Statement {
	public:
		Return_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> ret_expression;
	};

	class Expression_Statement : public Statement {
	public:
		Expression_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		Expression_Statement(std::shared_ptr<Expression> sub, std::shared_ptr<VarMap> scope)
			: Statement(scope), maybe_sub(sub) {}
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> maybe_sub;
	};

	class Compount_Statement : public Statement {
	public:
		Compount_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
	private:
		std::vector<std::shared_ptr<Statement>> statement_list;
		VarMap::Scope_Id scope_id;
	};

	class Declaration_Statement : public Statement {
	public:
		Declaration_Statement(const std::string& name, std::shared_ptr<VarMap> scope) : Statement(scope), var_name(name) {}
		Declaration_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
	private:
		std::string var_name;
	};

	class If_Statement : public Statement {
	public:
		If_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> condition;
		bool has_else;
		std::shared_ptr<Statement> true_statement;
		std::shared_ptr<Statement> false_statement;
	};

	class While_Statement : public Statement {
	public:
		While_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> condition;
		std::shared_ptr<Statement> contents;
	};

	class Do_While_Statement : public Statement {
	public:
		Do_While_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
	private:
		std::shared_ptr<Expression> condition;
		std::shared_ptr<Statement> contents;
	};

	class For_Statement : public Statement {
	public:
		For_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
	private:
		std::vector<std::shared_ptr<Statement>> maybe_set_up_statements;
		std::shared_ptr<Expression> maybe_condition_statement;
		std::shared_ptr<Expression> maybe_end_of_loop_expression;
		std::shared_ptr<Statement> contents;
		VarMap::Scope_Id scope_id;
	};

	class Break_Statement : public Statement {
	public:
		Break_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
	};

	class Continue_Statement : public Statement {
	public:
		Continue_Statement(const ParseNode& node, std::shared_ptr<VarMap> scope);
		std::string generate_code() const override;
	};

	class Function : public Compilable {
	public:
		enum class Parse_Type {
			DECLARATION,
			DEFINITION,
		};
		Function(const ParseNode& node, Environment* env, Parse_Type type);

		virtual ~Function() {};
		std::string generate_code() const;

		std::string get_name() const { return name; }
		bool signature_matches(const Function& other) const;
		bool signature_matches(const std::string& other_name, std::vector<std::shared_ptr<Expression>> other_args) const;
		bool is_defined() const { return is_fn_defined; }
		std::shared_ptr<Type> get_return_type() const { return return_type; }
	private:
		struct Arg_Type {
			std::shared_ptr<Type> type;
			std::string name;
			std::shared_ptr<Expression> maybe_init_value;
		};

		Environment* env;
		std::shared_ptr<VarMap> scope;

		std::shared_ptr<Type> return_type;
		std::string name;

		std::vector<Arg_Type> arguments;
		bool is_fn_defined; // if not defined, contents will be empty
		// TODO track whether function is static and not called, and delete if not
		std::shared_ptr<Compount_Statement> contents;
	};

	class Environment {
	public:
		Environment() = default;
		~Environment() = default;
		Environment(const Environment&&) = delete; // no move or copy

		void add_function(std::shared_ptr<Function> function);
		void check_function(
			const std::string& name,
			const std::vector<std::shared_ptr<Expression>>& args);
		std::shared_ptr<Type> get_function_return_type(const std::string& name);
		std::string generate_code();

		void create_static_var(const Declaration& declaration);
		std::shared_ptr<Variable> get_static_var(const std::string& name);

		Label_Maker label_maker;
		bool used_mult = false;
		bool used_div = false;
		bool used_mod = false;

	private:
		using Struct_Map_Type = std::map<std::string, std::shared_ptr<Type>>;
		Struct_Map_Type struct_map;

		struct Global_Symbol {
			std::string name;
			bool is_declared = false;

			enum class Global_Symbol_Type {
				FUNCTION,
				VARIABLE,
			};
			Global_Symbol_Type type;

			std::shared_ptr<Function> function;
			Static_Var static_var;
		};
		using Global_Symbol_Table_Type = std::vector<Global_Symbol>;
		Global_Symbol_Table_Type global_symbol_table;
		Global_Symbol* get_symbol_with_name(const std::string& name);
		Global_Symbol* get_declared_symbol_with_name(const std::string& name);
	};

	class AST {
	public:
		AST(const ParseNode& root);
		std::string generate_code();
	private:
		Environment env;
		std::shared_ptr<VarMap> global_scope;
	};
}
