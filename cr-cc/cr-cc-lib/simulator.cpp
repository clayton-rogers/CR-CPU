#include "simulator.h"

#include <exception>
#include <stdexcept>

enum SIM_OPCODES {
	ADD,
	SUB,
	AND,
	OR,
	XOR,
	SHIFT,
	LOAD,
	STORE,
	MOV,
	JMP,
	LOADI,
	IN_OUT_PUSH_POP,
	CALL_RET,
	LOADA,
	HALT,
	NOP,
};

static int ALU(int opcode, int shift_dir, int data1, int data2) {
	switch (opcode) {
	case ADD: return data1 + data2;
	case SUB: return data1 - data2;
	case AND: return data1 & data2;
	case OR:  return data1 | data2;
	case XOR: return data1 ^ data2; // WOW xor!
	case SHIFT:
		switch (shift_dir) {
		case 0: return data1 >> data2;
		case 1: return data1 << data2;
		default: throw std::out_of_range("Simulator::ALU: Shift dir must be 0 or 1");
		}
	case MOV: return data1;
	default: return 0;
	}
}

void Simulator::step() {
	if (is_halted) { return; }

	const std::string& str_inst = instructions.at(pc);
	inst = to_uint(str_inst);
	if (pc + 1 < instructions.size()) {
		auto str_next_inst = instructions.at(pc + 1);
		next_inst = to_uint(str_next_inst);
	} else {
		next_inst = 0x0000;
	}

	const int opcode = (inst & 0xF000) >> 12;
	const int extra = (inst & 0x0F00) >> 8;
	const int extra_high = (inst & 0x0C00) >> 10;
	const int extra_low = (inst & 0x0300) >> 8;
	const int constant = (inst & 0x00FF) >> 0;

	const std::uint16_t signed_constant = static_cast<std::uint16_t>((constant & 0x80) ? constant - 256 : constant);
	const std::uint16_t full_addr = static_cast<std::uint16_t>((addr << 8) + constant);

	const int input1 = (opcode == MOV) ? get_reg(extra_low) : get_reg(extra_high);
	int input2 = 0;
	switch (opcode) {
	case ADD:
	case SUB:
	case AND:
	case OR:
	case XOR:
	{
		if (extra_low == 3) {
			input2 = constant;
		} else {
			input2 = get_reg(extra_low);
		}
		break;
	}
	case SHIFT:
	{
		if (extra_low & 0x2) {
			input2 = constant;
		} else {
			input2 = rb;
		}
		break;
	}
	default:
		break;
	}

	const std::uint16_t ALU_OUT = static_cast<std::uint16_t>(ALU(opcode, (extra_low & 0x1), input1, input2));

	std::uint16_t& dest_reg = get_reg(extra_high);

	int ram_addr = 0;
	switch (extra_low) {
	case 0: ram_addr = signed_constant + bp; break;
	case 1: ram_addr = signed_constant + sp; break;
	case 2: ram_addr = full_addr; break;
	case 3: ram_addr = constant; break;
	default: throw std::out_of_range("Simulator::step(): ram_addr extra low must be in range 0 .. 3");
	}

	const std::uint16_t jmp_location = static_cast<std::uint16_t>((extra_low & 0x2) ? full_addr : pc + signed_constant);


	switch (opcode) {
	case ADD:
	case SUB:
	case AND:
	case OR:
	case XOR:
	case SHIFT:
		dest_reg = ALU_OUT;
		pc++;
		break;
	case LOAD:
	{
		if (state == 1) {
			state = 0;
			++pc;
			dest_reg = ram.at(ram_addr);
		} else {
			++state;
		}
		break;
	}
	case STORE:
		ram.at(ram_addr) = get_reg(extra_high);
		++pc;
		break;
	case MOV:
		dest_reg = get_reg(extra_low);
		++pc;
		break;
	case JMP:
	{
		if (state == 1) {
			state = 0;
			++pc;
			bool jump = false;
			switch (extra_high) {
			case 0: jump = true; break;
			case 1: jump = ra == 0; break;
			case 2: jump = ra != 0; break;
			case 3: jump = !(ra & 0x8000); break;
			default: throw std::out_of_range("Simulator::step(): type of jump, extra high must be in range 0 .. 3");
			}
			if (jump) {
				pc = jmp_location;
			}
		} else {
			++state;
		}
		break;
	}
	case LOADI:
	{
		if (extra_low & 0x2) {
			get_reg(extra_high) = static_cast<std::uint16_t>((constant << 8) + (get_reg(extra_high) & 0x00FF));
		} else {
			get_reg(extra_high) = static_cast<std::uint16_t>(constant);
		}
		++pc;
		break;
	}
	case IN_OUT_PUSH_POP:
	{
		++pc;
		switch (extra_low) {
		case 0: get_reg(extra_high) = input; break;
		case 1: output = get_reg(extra_high); break;
		case 2: ram.at(sp--) = get_reg(extra_high); break;
		case 3:
		{
			--pc;
			if (state == 1) {
				state = 0;
				++pc;
				get_reg(extra_high) = ram.at(++sp);
			} else {
				++state;
			}
			break;
		}
		default: throw std::out_of_range("Simulator::step(): extra_low must be 0 .. 3");
		}
		break;
	}
	case CALL_RET:
	{
		switch (extra_low) {
		case 0: // call
		{
			if (state == 1) {
				state = 0;
				ram.at(sp--) = pc;
				pc = full_addr;
			} else {
				++state;
			}
			break;
		}
		case 1: // rcall
		{
			if (state == 1) {
				state = 0;
				ram.at(sp--) = pc;
				pc += signed_constant;
			} else {
				++state;
			}
			break;
		}
		case 2: // ret
		case 3:
		{
			if (state == 3) {
				state = 0;
				pc = ram.at(++sp)+1;
			} else {
				++state;
			}
			break;
		}
		}
		break;
	}
	case LOADA:
		addr = static_cast<std::uint8_t>(constant);
		++pc;
		break;
	case HALT:
		is_halted = true;
		break;
	case NOP:
		++pc;
		break;
	default:
		throw std::out_of_range("Simulator::step(): Got unknown opcode: " + opcode);
	}

}

std::uint16_t Simulator::to_uint(const std::string& instruction)
{
	const int value = std::stoi(instruction, 0, 16);

	return static_cast<std::uint16_t>(value);
}

std::uint16_t& Simulator::get_reg(int index)
{
	switch (index) {
	case 0: return ra;
	case 1: return rb;
	case 2: return bp;
	case 3: return sp;
	default: throw std::out_of_range("Simulator::get_reg(): reg index must be 0 .. 3, actual: " + index);
	}
}
