#include "AST.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

namespace AST {

	static constexpr int MAX_SHORT_JUMP = 128;

	static std::string output_byte(int a) {
		if (a < 0 || a > 0xFF) {
			throw std::logic_error("Cannot generate byte for " + a);
		}
		std::stringstream ss;
		ss << "0x"
			<< std::hex
			<< std::uppercase
			<< std::setfill('0')
			<< std::setw(2)
			<< a;
		return ss.str();
	}

	static std::string output_signed_byte(int a) {
		if (a < -128 || a > 127) {
			throw std::logic_error("Cannot generate signed byte for " + a);
		}
		const unsigned char out_val = static_cast<char>(a);
		std::stringstream ss;
		ss << "0x"
			<< std::hex
			<< std::uppercase
			<< std::setfill('0')
			<< std::setw(2)
			<< static_cast<int>(out_val);
		return ss.str();
	}

	// returns logical not of ra
	static std::string gen_logical_negation(std::shared_ptr<VarMap> scope) {
		std::stringstream ss;
		std::string l_zero = scope->env->label_maker.get_next_label();
		std::string l_end = scope->env->label_maker.get_next_label();
		ss << "jmp.r.z " << l_zero << " # logical negation\n";
		ss << "loadi ra, 0\n";
		ss << "jmp.r " << l_end << "\n";
		ss << l_zero << ":\n";
		ss << "loadi ra, 1\n";
		ss << l_end << ":\n";
		return ss.str();
	}

	// return the truthiness of ra
	static std::string gen_is_true(std::shared_ptr<VarMap> scope) {
		std::stringstream ss;
		std::string l_false = scope->env->label_maker.get_next_label();
		std::string l_end = scope->env->label_maker.get_next_label();
		ss << "jmp.r.z " << l_false << " # is true\n";
		ss << "loadi ra, 1\n";
		ss << "jmp.r " << l_end << "\n";
		ss << l_false << ":\n";
		ss << "loadi ra, 0\n";
		ss << l_end << ":\n";
		return ss.str();
	}

	// returns 1 if ra > 0
	static std::string gen_is_positive(std::shared_ptr<VarMap> scope) {
		std::stringstream ss;
		std::string l_true = scope->env->label_maker.get_next_label();
		std::string l_end = scope->env->label_maker.get_next_label();
		ss << "jmp.r.gz " << l_true << " # is positive\n";
		ss << "loadi ra, 0\n";
		ss << "jmp.r " << l_end << "\n";
		ss << l_true << ":\n";
		ss << "loadi ra, 1\n";
		ss << l_end << ":\n";
		return ss.str();
	}

	// Returns the approx number of instructions in a section of code
	static int get_code_length(const std::string code) {
		const char* current_char = code.c_str();
		const int size = static_cast<int>(code.size());
		int count = 0;
		for (int i = 0; i < size; ++i) {
			if (*current_char == '\n') {
				++count;
			}
			++current_char;
		}
		return count;
	}

	static std::string expression_to_byte_array(std::shared_ptr<Expression> expression) {

		// For now will only support numeric constants.
		// In the future should support any const expression
		// And symbolic references
		// Also arrays, struct, etc.
		// TODO initialiser

		Expression* exp_ptr = expression.get();
		Constant_Expression* const_exp_ptr = dynamic_cast<Constant_Expression*>(exp_ptr);
		if (nullptr == const_exp_ptr) {
			throw std::logic_error("Static initialisation must be a constant expression");
		}

		return std::to_string(const_exp_ptr->get_value());
	}

	enum class Jump_Type {
		unconditional,
		if_zero_false,
		if_not_zero_true,
		if_greater_equal_zero,
	};
	std::string operator+(std::string lhs, Jump_Type rhs) {
		std::vector<std::string> map =
		{
			"",
			".z",
			".nz",
			".gz",
		};

		return lhs + map.at(static_cast<int>(rhs));
	}

	// Generates a long or short jump depending on the code size
	static std::string gen_jump(std::string label,
								Jump_Type type_of_jmp,
								std::string comment,
								int jump_size) {
		std::string jump;
		if (jump_size > MAX_SHORT_JUMP) {
			jump += "loada " + label + "\n";
			jump += "jmp" + type_of_jmp + " " + label + " " + comment + "\n";
		} else {
			jump += "jmp.r" + type_of_jmp + " " + label + " " + comment + "\n";
		}

		return jump;
	}

	std::string Unary_Expression::generate_code() const {
		std::stringstream ss;

		// calculate the sub expression and leave it in RA
		ss << sub->generate_code();

		switch (type) {
		case Unary_Type::negation:
			// to negate must invert all bits and then add one
			ss << "loadi rb, 0xFF # negate RA\n";
			ss << "loadi.h rb, 0xFF\n";
			ss << "xor ra, rb\n";
			//ss << "not ra, 0xFF \n"; TODO add not to CPU
			ss << "add ra, 1\n";
			break;
		case Unary_Type::bitwise_complement:
			ss << "loadi rb, 0xFF # complement RA\n";
			ss << "loadi.h rb, 0xFF\n";
			ss << "xor ra, rb\n";
			//ss << "not ra, 0xFF # complement RA\n"; TODO add not to CPU
			break;
		case Unary_Type::logical_negation:
		{
			ss << gen_logical_negation(scope);
			break;
		}

		default:
			throw std::logic_error("Unary generate code: should never happen");
		}

		return ss.str();
	}

	std::string Constant_Expression::generate_code() const {
		// If the constant is small, we can do it in one load, else two
		std::stringstream ss;
		ss << "loadi ra, " << output_byte(constant_value & 0xFF) << " # load const value: " << constant_value << '\n';
		if ((constant_value & 0x00FF) != constant_value) {
			// If the constant isn't small, we have to do two loads
			ss << "loadi.h ra, " << output_byte(constant_value >> 8) << '\n';
		}

		return ss.str();
	}

	std::string Binary_Expression::generate_code() const {
		std::stringstream ss;

		// Generate code for the left hand side
		ss << sub_left->generate_code();

		// If exp 1 is true and op is || then skip exp 2
		// If exp 1 is false(0) and op is && then skip exp2
		std::string label_after = scope->env->label_maker.get_next_label();
		if (type == Bin_Type::logical_or) {
			ss << "jmp.r.nz " << label_after << " # short circuit or\n";
		}
		if (type == Bin_Type::logical_and) {
			ss << "jmp.r.z " << label_after << " # short circuit and\n";
		}

		// Push onto stack and let the stack tracker know
		ss << scope->push_reg("ra") + "\n";

		// Generate code for the right hand side
		ss << sub_right->generate_code();

		// Move to rb and restore left hand side
		ss << "mov rb, ra\n";
		ss << scope->pop_reg("ra") + "\n";

		// Perform actual binary operation
		switch (type) {
		case Bin_Type::addition:
			ss << "add ra, rb # binary exp add\n";
			break;
		case Bin_Type::subtraction:
			ss << "sub ra, rb # binary exp sub\n";
			break;
		case Bin_Type::multiplication:
			// mult and div are not instructions but function calls
			// if they're used we have to make sure there's an .external
			// directive for them
			scope->env->used_mult = true;
			ss << "loada .__mult\n";
			ss << "call .__mult\n";
			break;
		case Bin_Type::division:
			// mult and div are not instructions but function calls
			// if they're used we have to make sure there's an .external
			// directive for them
			scope->env->used_div = true;
			ss << "loada .__div\n";
			ss << "call .__div\n";
			break;
		case Bin_Type::remainder:
			// mod is also a function and not an instruction
			scope->env->used_mod = true;
			ss << "loada .__rem\n";
			ss << "call .__rem\n";
			break;
		case Bin_Type::logical_and:
			ss << "and ra, rb # binary exp logical and\n";
			break;
		case Bin_Type::logical_or:
			ss << "or ra, rb # binary exp logical or\n";
			break;
		case Bin_Type::equal:
			// A == B is the same as !(A - B)
			// if A == B then A-B => 0 then !0 is 1
			ss << "sub ra, rb # binary exp logical equal\n";
			ss << gen_logical_negation(scope);
			break;
		case Bin_Type::not_equal:
			// if A == B is the same as !(A - B) from above
			// then A != B is the same as !!(A - B) which is just (A - B)
			ss << "sub ra, rb # binary exp logical not equal\n";
			ss << gen_is_true(scope); // probably not required
			break;
		case Bin_Type::less_than:
			// A < B is (B - A - 1) >= 0
			ss << "sub rb, ra # binary less than\n";
			ss << "mov ra, rb\n";
			ss << "sub ra, 1\n";
			ss << gen_is_positive(scope);
			break;
		case Bin_Type::less_than_or_equal:
			// A <= B is (B - A) >= 0
			ss << "sub rb, ra # binary less than equal\n";
			ss << "mov ra, rb\n";
			ss << gen_is_positive(scope);
			break;
		case Bin_Type::greater_than:
			// A > B  is (A - B - 1) >= 0
			ss << "sub ra, rb # binary greater than\n";
			ss << "sub ra, 1\n";
			ss << gen_is_positive(scope);
			break;
		case Bin_Type::greater_than_or_equal:
			// A >= B is (A - B) >= 0
			ss << "sub ra, rb # binary greater than equal\n";
			ss << gen_is_positive(scope);
			break;
		case Bin_Type::shift_left:
			ss << "shftl ra, rb # binary shift left\n";
			break;
		case Bin_Type::shift_right:
			ss << "shftr ra, rb # binary shift right\n";
			break;
		default:
			throw std::logic_error("Should never get here binary_expression::generate_code");
		}

		// In case of short circuit, skip the operation:
		if (type == Bin_Type::logical_and || type == Bin_Type::logical_or) {
			ss << label_after << ": # short circuit label\n";
		}

		// Result is now in ra
		return ss.str();
	}

	std::string Return_Statement::generate_code() const {
		std::stringstream ss;
		if (ret_expression) {
			ss << ret_expression->generate_code();
		}
		// Don't know how far the return will be so need to use long jmp
		const auto end_label = scope->env->label_maker.get_fn_end_label();
		ss << "loada " << end_label << "\n";
		ss << "jmp " << end_label << " # return <exp>\n";
		return ss.str();
	}

	std::string Expression_Statement::generate_code() const {
		// Expression statement may be empty, in which case it's a nop
		if (maybe_sub) {
			return maybe_sub->generate_code();
		} else {
			return "";
		}
	}

	std::string Compount_Statement::generate_code() const {
		std::string ret;

		scope->set_current_scope(scope_id);
		for (const auto& statement : statement_list) {
			ret += statement->generate_code();
		}
		scope->close_scope();

		return ret;
	}

	std::string Function::generate_code() const {
		std::stringstream ss;

		if (is_defined()) {

			this->scope->env->label_maker.set_fn_name(name);

			// Function header
			ss << "\n# " << name << '\n';
			// ss << arguments.... TODO
			ss << scope->env->label_maker.get_label_for_fn(name) << ":\n";
			ss << ".export " << name << "\n";

			// Calling convention is that the first two arguments will be in ra and rb in the case of the
			// compiler, these will be pushed immediately, but it allows for future optimization.
			// Any additonal argument will be pushed by the caller
			// RP will need to be saved as well when it's used in the future.
			const bool is_ra_argument = arguments.size() >= 1;
			const bool is_rb_argument = arguments.size() >= 2;

			ss << "push rb ";
			if (is_rb_argument) {
				ss << "# push arg rb\n";
			} else {
				ss << "# store caller rb\n";
			}
			ss << "push ra ";
			if (is_ra_argument) {
				ss << "# push arg ra\n";
			} else {
				ss << "# store caller ra\n";
			}

			// Allocated space on the stack for vars in scope
			ss << scope->gen_scope_entry();
			// Code for contents
			ss << contents->generate_code();
			// Exit block
			ss << "loadi ra, 0\n"; // In case funtion runs off the end, it should return 0.
			ss << scope->env->label_maker.get_fn_end_label() << ":\n";
			ss << scope->gen_scope_exit();
			// From above, we always push two int onto the stack, so we always have to remove those
			// but depending on what they are, we may have to restore them or discard them.
			if (arguments.size() >= 2) {
				// both ra and rb were args, so just drop them
				ss << "add sp, 2 # drop pushed ra, and rb\n";
			} else if (arguments.size() == 1) {
				// restore rb, drop arg ra
				ss << "add sp, 1 # drop arg ra\n";
				ss << "pop rb # restore caller rb\n";
			} else if (arguments.size() == 0 && return_type->get_size() == 0) {
				// restore caller ra and rb
				ss << "pop ra # restore caller ra\n";
				ss << "pop rb # restore caller rb\n";
			} else if (arguments.size() == 0) {
				// has a return type, so return is in ra, so don't have to restore ra
				ss << "add sp, 1 # drop ra because we're returning a value\n";
				ss << "pop rb # restore caller rb\n";
			} else {
				throw std::logic_error("Should never get here: unable to determine function epilogue");
			}
			ss << "ret\n";
		} else {
			// If the function has not been defined, assume this is an externally declared function
			ss << ".extern " + this->name << "\n";
		}

		return ss.str();
	}

	std::string AST::generate_code() {
		std::stringstream ss;
		ss << '\n';

		// generate any required constants, i.e. memory layouts???

		// Defer code gen to environment
		ss << env.generate_code();

		return ss.str();
	}

	std::string Environment::generate_code() {
		std::stringstream ss;

		// generate code for all global symbols (functions and vars)
		for (auto& symbol : global_symbol_table) {
			symbol.is_declared = true;
			switch (symbol.type) {
				case Global_Symbol::Global_Symbol_Type::FUNCTION:
					ss << symbol.function->generate_code();
					break;
				case Global_Symbol::Global_Symbol_Type::VARIABLE:
					ss << "\n";
					ss << ".static " << symbol.static_var.declaration.variable->type->get_size()
						<< " " << symbol.static_var.declaration.variable->identifier << " ";
					if (symbol.static_var.declaration.initialiser) {
						ss << expression_to_byte_array(symbol.static_var.declaration.initialiser);
					}
					ss << "\n";
					break;
			}
		}

		// generate extern for mult and div if needed
		// TODO probably proper way to do this is to add them to the function map
		ss << "\n";
		if (used_mult) {
			ss << ".extern __mult\n";
		}
		if (used_div) {
			ss << ".extern __div\n";
		}
		if (used_mod) {
			ss << ".extern __rem\n";
		}

		return ss.str();
	}

	void Environment::check_function(
		const std::string& name,
		const std::vector<std::shared_ptr<Expression>>& args)
	{
		auto symbol = get_symbol_with_name(name);
		if (symbol == nullptr || symbol->type != Global_Symbol::Global_Symbol_Type::FUNCTION) {
			throw std::logic_error("Called a function that has not been declared: " + name);
		}
		
		auto function = symbol->function;

		if (!function->signature_matches(name, args)) {
			throw std::logic_error("Function call has mismatched signature: " + name);
		}
	}

	std::shared_ptr<Type> Environment::get_function_return_type(const std::string& name) {
		// This function should never be called before "check_function()" has,
		// so we should not need to check that the function exists. We will anyways

		auto symbol = get_symbol_with_name(name);
		if (symbol == nullptr || symbol->type != Global_Symbol::Global_Symbol_Type::FUNCTION) {
			throw std::logic_error("Should never happen: getting return type of non-fn: " + name);
		}

		auto function = symbol->function;

		return function->get_return_type();
	}

	std::string VarMap::gen_scope_entry() {
		std::stringstream ss;

		if (size_of_var_map != 0) {
			ss << "sub sp, " << output_byte(size_of_var_map) << " # stack entry\n";
		}
		{
			Scope_Id id = 0;
			for (const auto& scope : scopes) {
				// We actually want to output them in offset order, not name order like they are in the map
				struct Temp_Var_Map {
					int offset;
					std::string name;
					Temp_Var_Map(int offset, std::string name) : offset(offset), name(name) {}
				};
				std::vector<Temp_Var_Map> vars;
				for (const auto& var : scope.offset_map) {
					vars.emplace_back(var.second.offset, var.first);
				}
				std::sort(vars.begin(), vars.end(), [](Temp_Var_Map a, Temp_Var_Map b) {
					return a.offset > b.offset;
					});
				for (const auto& var : vars) {
					ss << "# sp + " << var.offset << " = " << var.name << "_" << id << "\n";
				}
				++id;
			}
		}

		return ss.str();
	}

	std::string VarMap::gen_scope_exit() {
		std::stringstream ss;

		const int total_size = size_of_var_map + stack_offset;

		if (size_of_var_map != 0) {
			ss << "add sp, " << output_byte(total_size) << " # stack exit\n";
			ss << "# locals size: " << size_of_var_map << " temp size: " << stack_offset << "\n";

		}

		if (stack_offset != 0) {
			throw std::logic_error("gen_scope_exit: failed to pop all temporaries off of stack, stack offset: "
				+ std::to_string(stack_offset));
		}

		return ss.str();
	}

	std::string VarMap::push_reg(std::string reg_name) {
		stack_offset++;
		return "push " + reg_name;
	}

	std::string VarMap::pop_reg(std::string reg_name) {
		stack_offset--;
		return "pop " + reg_name;
	}

	//std::string Local_Variable::gen_load(int current_sp_offset) {
	//	std::stringstream ss;

	//	int var_position = current_sp_offset + sp_offset;
	//	if (var_position > 0xff) {
	//		throw std::logic_error("could not generate var load, offset too great");
	//	}

	//	ss << "load.sp ra, " << output_byte(var_position) << " # load " << name;

	//	return ss.str();
	//}

	//std::string Local_Variable::gen_store(int current_sp_offset) {
	//	std::stringstream ss;

	//	int var_position = current_sp_offset + sp_offset;
	//	if (var_position > 0xff) {
	//		throw std::logic_error("could not generate var load, offset too great");
	//	}

	//	ss << "store.sp ra, " << output_byte(var_position) << " # store " << name;

	//	return ss.str();
	//}

	std::string Assignment_Expression::generate_code() const {
		std::stringstream ss;

		// calculate the result of the sub expression
		ss << exp->generate_code();

		// Result is now in ra, store in memory location
		if (is_pointer) {
			ss << scope->load_word(var_name, "rp");
			ss << "store.rp ra, 0x00 # store through pointer\n";
		} else if (is_array) {
			// Need to calculate the array offset
			ss << scope->push_reg("ra") + " # push result while we calc the array offset\n";
			ss << array_index_exp->generate_code();
			ss << scope->load_address(var_name, "rp");
			ss << "add rp, ra # calculate final array offset\n";
			ss << scope->pop_reg("ra") + " # get back expression\n";
			ss << "store.rp ra, 0x00 # store through array\n";
		} else {
			ss << scope->store_word(var_name, "ra");
		}

		return ss.str();
	}

	std::string Variable_Expression::generate_code() const {
		std::stringstream ss;

		switch (var_type) {
		case Variable_Type::reference:
			// need to load the address of the given identifier into RA
			ss << scope->load_address(var_name, "ra");
			break;
		case Variable_Type::dereference:
			// need to load the value from the given identifier into RP
			// then load the value it points to
			ss << scope->load_word(var_name, "rp");
			ss << "load.rp ra, 0x00 # dereference pointer\n";
			break;
		case Variable_Type::direct:
			// Emit instruction to load var directly into ra
			ss << scope->load_word(var_name, "ra");
			break;
		case Variable_Type::array:
			// Calculate the offset, then get the dereference
			ss << array_index->generate_code();
			ss << "mov rp, ra # move array offset to ptr reg\n";
			ss << scope->load_address(var_name, "ra");
			ss << "add rp, ra # calculate final array addr\n";
			ss << "load.rp ra, 0x00 # load array value " << var_name << "\n";
			break;
		default:
			throw std::logic_error("variable_expression::generate_code(): should never get here: default case");
		}

		return ss.str();
	}

	int VarMap::get_var_offset(const std::string& name, Scope_Id* found_id) {
		Scope_Id id = current_scope;
		while (id != NULL_SCOPE) {
			// Only catch vars that exist at the current or parents scopes
			// and which have been declared
			if (scopes.at(id).offset_map.count(name) == 1 &&
				scopes.at(id).offset_map.at(name).is_declared == true) {

				*found_id = id;
				return scopes.at(id).offset_map.at(name).offset + stack_offset;
			}
			id = scopes.at(id).parent;
		}

		// if we got this far then we did not find the var
		*found_id = MAGIC_NO_SCOPE;
		return 0; // return value doesn't matter
	}

	std::string VarMap::store_word(const std::string& name, const std::string& source_reg) {
		Scope_Id id;

		// Try for stack var first
		{
			int offset = get_var_offset(name, &id);
			if (id != MAGIC_NO_SCOPE) {
				return "store.sp " + source_reg + ", " + output_signed_byte(offset) +
					" # store " + name + "_" + std::to_string(id) + "\n";
			}
		}

		// Else try for global var
		{
			auto type = env->get_static_var(name);
			if (type != nullptr) {
				return "loada ." + name + " # store global setup\n" +
					"store " + source_reg + " ." + name + " # store global\n";
			}
		}

		// If we've gotten this far then the var doesn't exist
		throw std::logic_error("Referenced unknown var: " + name);
	}

	std::string VarMap::load_word(const std::string& name, const std::string& dest_reg) {
		Scope_Id id;

		// Try for stack var first
		{
			int offset = get_var_offset(name, &id);
			if (id != MAGIC_NO_SCOPE) {
				return "load.sp " + dest_reg + ", " + output_signed_byte(offset) +
					" # load " + name + "_" + std::to_string(id) + "\n";
			}
		}

		// Else try for global var
		{
			auto static_var = env->get_static_var(name);
			if (static_var != nullptr) {
				return "loada ." + name + " # load global setup\n" +
					"load " + dest_reg + " ." + name + " # load global\n";
			}
		}

		// If we've got this far then the var doesn't exist
		throw std::logic_error("Referenced unknown var: " + name);
	}

	std::string VarMap::load_address(const std::string& name, const std::string& dest_reg) {
		Scope_Id id;

		// Try for stack var first
		{
			int offset = get_var_offset(name, &id);
			if (id != MAGIC_NO_SCOPE) {
				if (offset > 0xFF) {
					throw std::logic_error("When getting address of " + name + " offset was out of range");
				}
				return
					"mov " + dest_reg + ", sp # get address of stack var " + name + "_" + std::to_string(id) + "\n"
					"add " + dest_reg + ", " + output_signed_byte(offset) + " # add var offset\n";
			}
		}

		// Else try for global var
		{
			auto static_var = env->get_static_var(name);
			if (static_var != nullptr) {
				return
					"loadi " + dest_reg + ", ." + name + " # load address of global low byte\n" +
					"loadi.h " + dest_reg + ", ." + name + " # load address of global high byte\n";
			}
		}

		// If we've got this far then the var doesn't exist
		throw std::logic_error("Tried to get address of unknown var: " + name);
	}

	std::string If_Statement::generate_code() const {
		std::stringstream ss;

		// evaluate the condition
		ss << condition->generate_code();
		if (has_else) {
			const auto else_label = scope->env->label_maker.get_next_label();
			const auto end_label = scope->env->label_maker.get_next_label();
			const auto true_code = true_statement->generate_code();
			const auto true_code_size = get_code_length(true_code);
			const auto false_code = false_statement->generate_code();
			const auto false_code_size = get_code_length(false_code);

			ss << gen_jump(else_label, Jump_Type::if_zero_false, "# if else statement", true_code_size);
			ss << true_code;
			ss << gen_jump(end_label, Jump_Type::unconditional, "# skip else", false_code_size);
			ss << else_label << ": # else\n";
			ss << false_code;
			ss << end_label << ": # end if\n";
		} else {
			const auto end_label = scope->env->label_maker.get_next_label();
			const auto true_code = true_statement->generate_code();
			const auto true_code_size = get_code_length(true_code);

			ss << gen_jump(end_label, Jump_Type::if_zero_false, "# if statement", true_code_size);
			ss << true_code;
			ss << end_label << ": # end if\n";
		}

		return ss.str();
	}

	std::string Conditional_Expression::generate_code() const {
		std::stringstream ss;

		// WTF are you doing if you need a long jmp for a ternary?
		// but we'll implement it anyways since we're here.

		// This should be very similar to the if statement above
		ss << condition->generate_code();
		const auto else_label = scope->env->label_maker.get_next_label();
		const auto end_label = scope->env->label_maker.get_next_label();
		const auto true_code = true_exp->generate_code();
		const auto true_code_size = get_code_length(true_code);
		const auto false_code = false_exp->generate_code();
		const auto false_code_size = get_code_length(false_code);

		ss << gen_jump(else_label, Jump_Type::if_zero_false, "# conditional expression", true_code_size);
		ss << true_code;
		ss << gen_jump(end_label, Jump_Type::unconditional, "# skip else", false_code_size);
		ss << else_label << ": # else\n";
		ss << false_code;
		ss << end_label << ": # end conditional expression\n";

		return ss.str();
	}

	std::string Function_Call_Expression::generate_code() const {
		std::stringstream ss;

		// TODO we need to modify this if types other than int exist

		auto number_args = arguments.size();

		// Calculate and setup args for call
		while (number_args > 2) {
			ss << arguments.at(number_args - 1)->generate_code();
			ss << scope->push_reg("ra") + "\n";
			--number_args;
		}
		if (number_args == 2) {
			ss << arguments.at(number_args - 1)->generate_code();
			ss << scope->push_reg("ra") + "\n";
			--number_args;
			ss << arguments.at(number_args - 1)->generate_code();
			ss << scope->pop_reg("rb") + "\n";
		} else if (number_args == 1) {
			ss << arguments.at(number_args - 1)->generate_code();
		} else if (number_args == 0) {
			// nothing to do, no args to setup
		} else {
			throw std::logic_error("Should never get here: called Function_Call_Expression::generate_code"
				" with weird number of arguments: " + std::to_string(number_args));
		}

		// The actual funtion call proper
		ss << "loada ." << name << " # function call\n";
		ss << "call ." << name << "\n";

		// Clean up after the function call
		number_args = arguments.size();
		if (number_args > 2) {
			number_args -= 2;
			// We use "pop_reg" to fix the stack accounting we setup prior to the function
			// since we don't need the values, as an optimization, we ignore the return
			// of pop_reg and instead add the correct amount to the stack
			for (int i = 0; i < static_cast<int>(number_args); ++i) {
				scope->pop_reg("");
			}
			ss << "add sp, " << output_signed_byte(static_cast<int>(number_args)) << " # clean up after function call\n";
		}

		return ss.str();
	}

	std::string Declaration_Statement::generate_code() const {
		// Don't actually do anything for a declaration, just
		// mark the var as available to the subsequent code.
		scope->declare_var(var_name);
		return "";
	}

	std::string While_Statement::generate_code() const {
		std::stringstream ss;

		VarMap::Loop_Labels label;
		label.top = scope->env->label_maker.get_next_label();
		label.after = scope->env->label_maker.get_next_label();

		scope->push_loop(label);

		// Check the condition, if false(zero) skip to end
		ss << label.top << ": # while statement\n";
		ss << condition->generate_code();

		// We need to use a long jmp if the size of the inside is too long
		const auto while_contents = contents->generate_code();
		const auto while_contents_size = get_code_length(while_contents);
		ss << gen_jump(label.after, Jump_Type::if_zero_false, "", while_contents_size);
		ss << while_contents;
		ss << gen_jump(label.top, Jump_Type::unconditional, " # end of while statement", while_contents_size);
		ss << label.after << ":\n";
		
		scope->pop_loop();

		return ss.str();
	}

	std::string Do_While_Statement::generate_code() const {
		std::stringstream ss;

		VarMap::Loop_Labels label;
		label.top = scope->env->label_maker.get_next_label();
		label.after = scope->env->label_maker.get_next_label();

		scope->push_loop(label);

		// Check the condition, if false(zero) skip to end
		const auto do_while_contents = contents->generate_code() + condition->generate_code();
		const auto do_while_contents_size = get_code_length(do_while_contents);
		
		ss << label.top << ": # do while statement\n";
		ss << do_while_contents;
		ss << "jmp.r.z " << label.after << "\n";
		ss << gen_jump(label.top, Jump_Type::unconditional, "# end of do while statement", do_while_contents_size);
		ss << label.after << ":\n";

		scope->pop_loop();

		return ss.str();
	}

	std::string For_Statement::generate_code() const {
		std::stringstream ss;
		scope->set_current_scope(scope_id);

		// label.top is where a continue should go
		// label.after is where a break should go
		// in the case of a for loop, a continue should execute the optional increment expression (which is not at the top)
		VarMap::Loop_Labels label;
		std::string actual_top = scope->env->label_maker.get_next_label();
		label.top = scope->env->label_maker.get_next_label();
		label.after = scope->env->label_maker.get_next_label();

		ss << "# for loop\n";
		for (const auto& statement : maybe_set_up_statements) {
			ss << statement->generate_code();
		}

		scope->push_loop(label);

		const auto condition_code = maybe_condition_statement ? maybe_condition_statement->generate_code() : "loadi ra, 1\n";
		const auto condition_code_size = get_code_length(condition_code);
		const auto contents_code = contents->generate_code();
		const auto contents_code_size = get_code_length(contents_code);
		const auto end_of_loop_code = maybe_end_of_loop_expression ? maybe_end_of_loop_expression->generate_code() : "";
		const auto end_of_loop_code_size = get_code_length(end_of_loop_code);

		ss << actual_top << ": # top of for loop\n";
		ss << condition_code;
		ss << gen_jump(label.after, Jump_Type::if_zero_false, "# for loop condition", contents_code_size + end_of_loop_code_size);
		ss << contents_code;
		ss << label.top << ": # for increment expression\n";
		ss << end_of_loop_code;
		ss << gen_jump(actual_top, Jump_Type::unconditional, "# end of for statement", condition_code_size + contents_code_size + end_of_loop_code_size);
		ss << label.after << ":\n";

		scope->pop_loop();

		scope->close_scope();

		return ss.str();
	}

	std::string Break_Statement::generate_code() const {
		// No way to know how far a break jump has to go, so always long jump
		std::string ret;
		ret += "loada " + scope->get_after_label() + "\n";
		ret += "jmp " + scope->get_after_label() + " # break\n";
		return ret;
	}

	std::string Continue_Statement::generate_code() const {
		// No way to know how far a continue jump has to go, so always long jump
		std::string ret;
		ret += "loada " + scope->get_top_label() + "\n";
		ret += "jmp " + scope->get_top_label() + " # continue\n";
		return ret;
	}
}
