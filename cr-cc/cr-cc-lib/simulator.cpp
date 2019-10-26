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
	PUSH_POP,
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

	bus->write_strobe = false; // will be overwritten if an actual write occurs

	const std::uint16_t inst = (state == 0) ? bus->read_data : cached_ins;
	const std::uint16_t next_cached_inst = bus->read_data;

	const std::uint16_t opcode = (inst & 0xF000) >> 12;
	const std::uint16_t extra = (inst & 0x0F00) >> 8;
	const std::uint16_t extra_high = (inst & 0x0C00) >> 10;
	const std::uint16_t extra_low = (inst & 0x0300) >> 8;
	const std::uint16_t constant = (inst & 0x00FF) >> 0;

	const std::uint16_t signed_constant = static_cast<std::uint16_t>((constant & 0x80) ? constant | 0xFF00 : constant);
	const std::uint16_t full_addr = static_cast<std::uint16_t>((addr << 8) + constant);

	const bool is_rel = !(extra_low & 0x02);

	// Calculate CJ address (call/jump)
	const std::uint16_t rel_cj_addr = pc + signed_constant;
	const std::uint16_t cj_addr = is_rel ? rel_cj_addr : full_addr;

	// Popped addr for ret inst
	const std::uint16_t popped_addr = bus->read_data;

	// Calculate new PC's
	const std::uint16_t calced_pc = (opcode == CALL_RET && (extra_low & 0x01)) ? popped_addr : cj_addr;
	const std::uint16_t inc_pc = pc + 1;

	// Calculate load address
	std::uint16_t load_addr = 0;
	{
		const std::uint16_t ret_addr = sp;

		const std::uint16_t base_addr = (extra_low & 0x01) ? sp : rp;
		const std::uint16_t offset_addr = base_addr + signed_constant;
		const std::uint16_t load_inst_addr = (extra_low & 0x02) ? full_addr : offset_addr;

		load_addr = (opcode == CALL_RET || (opcode == PUSH_POP && extra_low == 0x01)) ?
			ret_addr : load_inst_addr;
	}

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

	bool should_jump = false;
	switch (extra_high) {
		case 0: should_jump = true; break;
		case 1: should_jump = ra == 0; break;
		case 2: should_jump = ra != 0; break;
		case 3: should_jump = !(ra & 0x8000); break;
		default: throw std::out_of_range("Simulator::step(): type of jump, extra high must be in range 0 .. 3");
	}

	switch (opcode) {
	case ADD:
	case SUB:
	case AND:
	case OR:
	case XOR:
	case SHIFT:
		dest_reg = ALU_OUT;
		break;
	case LOAD:
	{
		if (state == 0) {
			// do nothing, read address will automatically be ram addr
		} else { // state == 1
			dest_reg = bus->read_data; // retrieve the data that was previously requested
		}
		break;
	}
	case STORE:
		bus->write_addr = load_addr;
		bus->write_data = get_reg(extra_high);
		bus->write_strobe = true;
		break;
	case MOV:
		dest_reg = get_reg(extra_low);
		break;
	case JMP:
	{
		// Handled by combinatoric logic
		break;
	}
	case LOADI:
	{
		if (extra_low & 0x2) {
			get_reg(extra_high) = static_cast<std::uint16_t>((constant << 8) + (get_reg(extra_high) & 0x00FF));
		} else {
			get_reg(extra_high) = static_cast<std::uint16_t>(constant);
		}
		break;
	}
	case PUSH_POP:
	{
		switch (extra_low) {
		case 0:
		{
			bus->write_addr = --sp;
			bus->write_data = get_reg(extra_high);
			bus->write_strobe = true;
		}
		break;
		case 1:
		{
			if (state == 0) {
				// address to load from is combinatoric
			} else {
				get_reg(extra_high) = bus->read_data;
			}
			break;
		}
		default: throw std::out_of_range("Simulator::step(): extra_low must be 0 .. 3");
		}
		break;
	}
	case CALL_RET:
	{
		if (extra_low & 0x01) {
			// RET handled combinatoric
			if (state == 1) {
				sp++;
			}
		} else {
			// Call
			// Update of pc is combinatoric
			bus->write_addr = --sp;
			bus->write_data = inc_pc;
			bus->write_strobe = true;
		}
		break;
	} // end CALL_RET
	case LOADA:
		addr = static_cast<std::uint8_t>(constant);
		break;
	case HALT:
		is_halted = true;
		break;
	case NOP:
		break;
	default:
		throw std::out_of_range("Simulator::step(): Got unknown opcode: " + opcode);
	}

	// Calculate next PC
	std::uint16_t next_pc = 0;
	int next_state = 0;
	if ((opcode == JMP && should_jump) ||  // jmp == true
		(opcode == CALL_RET && (extra_low & 0x01) == 0) || // call
		(opcode == CALL_RET && extra_low & 0x01 && state == 1) // ret
	) {
		next_pc = calced_pc;
	} else if ((opcode == LOAD && state == 0) || // Load first cycle
		(opcode == CALL_RET && extra_low & 0x01 && state == 0) || // Ret first cycle
		(opcode == PUSH_POP && extra_low == 0x01 && state == 0) // Pop first cycle
	) {
		next_pc = pc;
		next_state = 1;
	} else {
		next_pc = pc + 1;
	}

	// Calculate read address
	if ((opcode == LOAD && state == 0) ||  // first cycle of load
		(opcode == CALL_RET && extra_low & 0x01 && state == 0) || // first cycle of ret
		(opcode == PUSH_POP && extra_low == 0x01 && state == 0)  // pop
	) {
		bus->read_addr = load_addr;
	} else {
		bus->read_addr = next_pc;
	}

	pc = next_pc;
	cached_ins = next_cached_inst;
	state = next_state;
}

std::uint16_t& Simulator::get_reg(int index)
{
	switch (index) {
	case 0: return ra;
	case 1: return rb;
	case 2: return rp;
	case 3: return sp;
	default: throw std::out_of_range("Simulator::get_reg(): reg index must be 0 .. 3, actual: " + index);
	}
}
