#include "AST.h"

#include <sstream>
#include <iomanip>

namespace AST {

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

	// returns logical not of ra
	static std::string gen_logical_negation(std::shared_ptr<Scope> scope) {
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
	static std::string gen_is_true(std::shared_ptr<Scope> scope) {
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
	static std::string gen_is_positive(std::shared_ptr<Scope> scope) {
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

	std::string Unary_Expression::generate_code() const {
		std::stringstream ss;

		// calculate the sub expression and leave it in RA
		ss << sub->generate_code();

		switch (type) {
		case Type::negation:
			// to negate must invert all bits and then add one
			ss << "loadi rb, 0xFF # negate RA\n";
			ss << "loadi.h rb, 0xFF\n";
			ss << "xor ra, rb\n";
			//ss << "not ra, 0xFF \n"; TODO add not to CPU
			ss << "add ra, 1\n";
			break;
		case Type::bitwise_complement:
			ss << "loadi rb, 0xFF # complement RA\n";
			ss << "loadi.h rb, 0xFF\n";
			ss << "xor ra, rb\n";
			//ss << "not ra, 0xFF # complement RA\n"; TODO add not to CPU
			break;
		case Type::logical_negation:
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
		if (type == Type::logical_or) {
			ss << "jmp.r.nz " << label_after << " # short circuit or\n";
		}
		if (type == Type::logical_and) {
			ss << "jmp.r.z " << label_after << " # short circuit and\n";
		}

		// Push onto stack and let the stack tracker know
		ss << scope->push_reg("ra");

		// Generate code for the right hand side
		ss << sub_right->generate_code();

		// Move to rb and restore left hand side
		ss << "mov rb, ra\n";
		ss << scope->pop_reg("ra");

		// Perform actual binary operation
		switch (type) {
		case Type::addition:
			ss << "add ra, rb # binary exp add\n";
			break;
		case Type::subtraction:
			ss << "sub ra, rb # binary exp sub\n";
			break;
		case Type::multiplication:
			ss << "loadi ra 0\n"; // "return 0" for unit tests
			ss << "halt # binary exp mult\n"; // TODO no multiplication
			break;
		case Type::division:
			ss << "loadi ra 0\n"; // "return 0" for unit tests
			ss << "halt # binary exp div\n"; // TODO no division
			break;
		case Type::logical_and:
			ss << "and ra, rb # binary exp logical and\n";
			break;
		case Type::logical_or:
			ss << "or ra, rb # binary exp logical or\n";
			break;
		case Type::equal:
			// A == B is the same as !(A - B)
			// if A == B then A-B => 0 then !0 is 1
			ss << "sub ra, rb # binary exp logical equal\n";
			ss << gen_logical_negation(scope);
			break;
		case Type::not_equal:
			// if A == B is the same as !(A - B) from above
			// then A != B is the same as !!(A - B) which is just (A - B)
			ss << "sub ra, rb # binary exp logical not equal\n";
			ss << gen_is_true(scope); // probably not required
			break;
		case Type::less_than:
			// A < B is (B - A - 1) >= 0
			ss << "sub rb, ra # binary less than\n";
			ss << "mov ra, rb\n";
			ss << "sub ra, 1\n";
			ss << gen_is_positive(scope);
			break;
		case Type::less_than_or_equal:
			// A <= B is (B - A) >= 0
			ss << "sub rb, ra # binary less than equal\n";
			ss << "mov ra, rb\n";
			ss << gen_is_positive(scope);
			break;
		case Type::greater_than:
			// A > B  is (A - B - 1) >= 0
			ss << "sub ra, rb # binary greater than\n";
			ss << "sub ra, 1\n";
			ss << gen_is_positive(scope);
			break;
		case Type::greater_than_or_equal:
			// A >= B is (A - B) >= 0
			ss << "sub ra, rb # binary greater than equal\n";
			ss << gen_is_positive(scope);
			break;
		default:
			throw std::logic_error("Should never get here binary_expression::generate_code");
		}

		// In case of short circuit, skip the operation:
		ss << label_after << ":\n";

		// Result is now in ra
		return ss.str();
	}

	std::string Return_Statement::generate_code() const {
		std::stringstream ss;
		if (ret_expression) {
			ss << ret_expression->generate_code();
		}
		ss << "jmp.r " << scope->env->label_maker.get_fn_end_label() << " # return <exp>\n";
		return ss.str();
	}

	std::string Expression_Statement::generate_code() const {
		return sub->generate_code();
	}

	std::string Compount_Statement::generate_code() const {
		std::string ret;

		for (const auto& statement : statement_list) {
			ret += statement->generate_code();
		}

		return ret;
	}

	std::string Function::generate_code() const {
		std::stringstream ss;

		this->scope->env->label_maker.set_fn_name(name);

		// Function header
		ss << "\n# " << name << '\n';
		// ss << arguments.... TODO
		ss << scope->env->label_maker.get_label_for_fn(name) << ":\n";
		// TODO any pushing of temp registers
		// TODO allocate args to registers/calling stack
		//ss << "push ra\n";
		//ss << "push rb\n";
		//ss << "push rp\n";
		// Allocated space on the stack for vars in scope
		ss << scope->gen_scope_entry();
		// Code for contents
		ss << contents->generate_code();
		// Exit block
		ss << "loadi ra, 0\n"; // In case funtion runs off the end, it should return 0.
		ss << scope->env->label_maker.get_fn_end_label() << ":\n";
		ss << scope->gen_scope_exit();
		//ss << "pop rp\n";
		//ss << "pop rb\n";
		//ss << "pop ra\n";
		ss << "ret\n";

		return ss.str();
	}

	std::string AST::generate_code() const {
		std::stringstream ss;
		ss << '\n';

		// For now assume all programs are userspace programs
		ss << ".text_offset 0x0200\n\n";

		// generate any required constants, i.e. memory layouts???

		// generate any global vars at global scope

		// generate entry into main
		ss << "loada ._main\n"; // Can't guarantee main is in range of a short jumpe
		ss << "call ._main\n";
		ss << "ret\n"; // return to the OS

		// generate code for each function
		for (const auto& fn : functions) {
			ss << fn->generate_code();
		}

		return ss.str();
	}

	const Type* Environment::get_type(std::string name) const {
		if (type_map.count(name) == 1) {
			return type_map.at(name);
		} else {
			throw std::logic_error("Failed to find type: " + name);
		}
	}
	void Environment::create_type(Type* type) {
		type_map[type->get_name()] = type;
	}

	void Scope::create_stack_var(const Type* type, std::string name) {
		int var_offset = size_of_scope;
		size_of_scope += type->get_size();

		// Double check that the var doesn't exist already
		for (const auto& var : vars) {
			if (var.name == name) {
				throw std::logic_error("Duplicate var with name: " + name);
			}
		}
		vars.push_back({ var_offset, name });
	}

	int Scope::get_var_offset(std::string name) {
		for (const auto& var : vars) {
			if (var.name == name) {
				// The variable's offset within the frame plus any temps stored on the stack
				return var.offset + stack_offset;
			}
		}
		throw std::logic_error("Referenced unknown variable: " + name);
	}

	std::string Scope::gen_scope_entry() {
		std::stringstream ss;

		if (size_of_scope != 0) {
			ss << "sub sp, " << output_byte(size_of_scope) << " # stack entry\n";
			for (const auto& var : vars) {
				ss << "# sp + " << var.offset << " = " << var.name << "\n";
			}
		}

		return ss.str();
	}

	std::string Scope::gen_scope_exit() {
		std::stringstream ss;

		const int total_size = size_of_scope + stack_offset;

		if (size_of_scope != 0) {
			ss << "add sp, " << output_byte(total_size) << " # stack exit\n";
			ss << "# locals size: " << size_of_scope << " temp size: " << stack_offset << "\n";

		}

		if (stack_offset != 0) {
			throw std::logic_error("gen_scope_exit: failed to pop all temporaries off of stack, stack offset: "
				+ std::to_string(stack_offset));
		}

		return ss.str();
	}

	std::string Scope::push_reg(std::string reg_name) {
		stack_offset++;
		return "push " + reg_name + "\n";
	}

	std::string Scope::pop_reg(std::string reg_name) {
		stack_offset--;
		return "pop " + reg_name + "\n";
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
		const int var_offset = scope->get_var_offset(var_name);
		ss << "store.sp ra, " + output_byte(var_offset) << " # store " << var_name << "\n";

		return ss.str();
	}

	std::string Variable_Expression::generate_code() const {
		std::stringstream ss;

		const int var_offset = scope->get_var_offset(var_name);
		ss << "load.sp ra, " << output_byte(var_offset) << " # load " << var_name << "\n";

		return ss.str();
	}
}
