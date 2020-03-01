#include "AST.h"

#include <sstream>
#include <iomanip>

namespace AST {

	static std::string output_byte(std::uint16_t a) {
		std::stringstream ss;
		ss << "0x"
			<< std::hex
			<< std::uppercase
			<< std::setfill('0')
			<< std::setw(2)
			<< a;
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
			std::string l_zero = scope->label_maker->get_next_label();
			std::string l_other = scope->label_maker->get_next_label();
			ss << "jmp.r.z " << l_zero << " # logical negation\n";
			ss << "loadi ra, 0\n";
			ss << "jmp.r " << l_other << "\n";
			ss << l_zero << ":\n";
			ss << "loadi ra, 1\n";
			ss << l_other << ":\n";
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

		// Push onto stack and let the stack tracker know
		ss << "push ra\n";
		scope->modify_stack_offset(1);

		// Generate code for the right hand side
		ss << sub_right->generate_code();

		// Move to rb and restore left hand side
		ss << "mov rb, ra\n";
		ss << "pop ra\n";
		scope->modify_stack_offset(-1);

		// Perform actual binary operation
		switch (type) {
		case Type::addition:
			ss << "add ra, rb # binary exp add\n";
			break;
		case Type::subtraction:
			ss << "sub ra, rb # binary exp sub\n";
			break;
		case Type::multiplication:
			ss << "halt # binary exp mult\n"; // TODO no multiplication
			break;
		case Type::division:
			ss << "halt # binary exp div\n"; // TODO no division
			break;
		}

		// Result is now in ra
		return ss.str();
	}

	std::string Return_Statement::generate_code() const {
		std::stringstream ss;
		if (ret_expression) {
			ss << ret_expression->generate_code();
		}
		ss << "jmp.r " << scope->label_maker->get_fn_end_label() << " # return <exp>\n";
		return ss.str();
	}

	std::string Code_Block::generate_code() const {
		std::string ret;

		ret += scope->gen_scope_entry();

		for (const auto& statement : statement_list) {
			ret += statement->generate_code();
		}

		ret += scope->gen_scope_exit();

		return ret;
	}

	std::string Function::generate_code() const {
		std::stringstream ss;

		this->scope->label_maker->set_fn_name(name);

		// Function header
		ss << "\n# " << name << '\n';
		// ss << arguments.... TODO
		ss << scope->label_maker->get_label_for_fn(name) << ":\n";
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
		ss << scope->label_maker->get_fn_end_label() << ":\n";
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

	std::string Scope::gen_scope_entry() {
		std::stringstream ss;

		int size_of_scope = 0;
		for (const auto& var : symbol_table) {
			size_of_scope += var.second->get_type()->get_size();
		}
		if (size_of_scope > 0xFF) {
			throw std::logic_error("Scope size too large!");
		}

		// If the scope is empty then there's nothing to do
		if (size_of_scope != 0) {
			ss << "sub sp, "
				<< output_byte(static_cast<std::uint16_t>(size_of_scope))
				<< " # scope size: "
				<< std::to_string(size_of_scope) << "\n";
			for (const auto& var : symbol_table) {
				size_of_scope -= var.second->get_type()->get_size();
				ss << "# sp + " << std::to_string(size_of_scope) << " : " << var.second->get_name() << "\n";
			}
		}

		return ss.str();
	}

	std::string Scope::gen_scope_exit() {
		if (stack_offset != 0) {
			throw std::logic_error("gen_scope_exit: failed to pop all temporaries off of stack, stack offset: "
				+ std::to_string(stack_offset));
		}

		std::stringstream ss;

		int size_of_scope = 0;
		for (const auto& var : symbol_table) {
			size_of_scope += var.second->get_type()->get_size();
		}
		if (size_of_scope > 0xFF) {
			throw std::logic_error("Scope size too large!");
		}

		// If the scope is empty then there's nothing to do
		if (size_of_scope != 0) {
			ss << "add sp, "
				<< output_byte(static_cast<std::uint16_t>(size_of_scope))
				<< " # reclaiming scope: "
				<< std::to_string(size_of_scope) << "\n";
		}

		return ss.str();
	}
}
