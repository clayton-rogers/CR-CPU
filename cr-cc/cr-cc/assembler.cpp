#include "assembler.h"

#include <map>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>
#include <cstdint>
#include <iomanip>

enum class opcode {
	ADD,
	SUB,
	AND,
	OR,
	XOR,
	SHFTL,
	SHFTR,
	LOAD,
	STORE,
	MOV,
	JMP,
	JIFZ,
	JINZ,
	JGZ,
	LOADC,
	LOADCH,
	OUT,
	HALT,
	NOP,
};

enum class argument_t {
	RA,
	RB,
	RC,
	RD,
	CONST,
	NONE,
};

struct argument {
	argument_t type = argument_t::RA;
	int value = 0;
};

static const int NUMBER_ARGS = 3;

struct instruction_type {
	opcode opcode = opcode::NOP;
	argument arg[NUMBER_ARGS];
};

struct instruction_info {
	// one destination and up to two arguments
	std::vector<argument_t> allowable[NUMBER_ARGS];
};

static const std::vector<argument_t> ALL_REG = { argument_t::RA, argument_t::RB, argument_t::RC, argument_t::RD };
// exclusion of RD from this list is deliberate
static const std::vector<argument_t> ANY_REG = { argument_t::RA, argument_t::RB, argument_t::RC, argument_t::CONST };
static const std::vector<argument_t> OPTIONAL_CONST = { argument_t::CONST, argument_t::NONE };
static const std::vector<argument_t> NO_ARG = { argument_t::NONE };

static const instruction_info STANDARD_ARITHMETIC = {
	ALL_REG,
   {argument_t::RA, argument_t::RC},
   {argument_t::RB, argument_t::CONST}
};

static const instruction_info NO_ARGS = {NO_ARG,NO_ARG,NO_ARG};

static std::map<opcode, instruction_info> instructions = {
	{opcode::ADD,  STANDARD_ARITHMETIC},
	{opcode::SUB,  STANDARD_ARITHMETIC},
	{opcode::AND,  STANDARD_ARITHMETIC},
	{opcode::OR,   STANDARD_ARITHMETIC},
	{opcode::XOR,  STANDARD_ARITHMETIC},
	{opcode::SHFTL, {ALL_REG, {argument_t::RB, argument_t::CONST},{argument_t::NONE}}},
	{opcode::SHFTR, {ALL_REG, {argument_t::RB, argument_t::CONST},{argument_t::NONE}}},
	{opcode::LOAD,  {ALL_REG, ANY_REG, OPTIONAL_CONST}},
	{opcode::STORE, {ALL_REG, ANY_REG, OPTIONAL_CONST}},
	{opcode::MOV,   {ALL_REG, ALL_REG, NO_ARG}},
	{opcode::JMP,   {ANY_REG, OPTIONAL_CONST, NO_ARG}},
	{opcode::JIFZ,  {ANY_REG, OPTIONAL_CONST, NO_ARG}},
	{opcode::JINZ,  {ANY_REG, OPTIONAL_CONST, NO_ARG}},
	{opcode::JGZ,   {ANY_REG, OPTIONAL_CONST, NO_ARG}},
	{opcode::LOADC, {ALL_REG, {argument_t::CONST}, NO_ARG}},
	{opcode::LOADCH,{ALL_REG, {argument_t::CONST}, NO_ARG}},
	{opcode::OUT, {ALL_REG, NO_ARG, NO_ARG}},
	{opcode::HALT,  NO_ARGS},
	{opcode::NOP,  NO_ARGS},
};

struct line_t {
	int line_number;
	std::string line;
	std::vector<std::string> tokens;
	instruction_type inst;
};

static std::string convert_line(const std::string& input) {
	std::string output;
	output.reserve(input.size());

	char last = ' ';
	for (int i = 0; i < input.size(); ++i) {
		char current = input.at(i);

		current = static_cast<char>(std::tolower(current));
		if (current == ',' || current == '\t') {
			current = ' ';
		}

		if (current == ' ' && last == ' ') {
			// don't add
		} else {
			output.append(&current, 1);
		}

		last = current;
	}

	return output;
}

static const std::map<std::string, opcode> instruction_map = {
	{"add", opcode::ADD},
	{"sub", opcode::SUB},
	{"and", opcode::AND},
	{"or", opcode::OR},
	{"xor", opcode::XOR},
	{"shftl", opcode::SHFTL},
	{"shftr", opcode::SHFTR},
	{"load", opcode::LOAD},
	{"store", opcode::STORE},
	{"mov", opcode::MOV},
	{"jmp", opcode::JMP},
	{"jifz", opcode::JIFZ},
	{"jinz", opcode::JINZ},
	{"jgz", opcode::JGZ},
	{"loadc", opcode::LOADC},
	{"loadch", opcode::LOADCH},
	{"out", opcode::OUT},
	{"halt", opcode::HALT},
	{"nop", opcode::NOP},

	// TODO add rest
};

static const std::map<std::string, argument_t> argument_map = {
	{"ra", argument_t::RA},
	{"rb", argument_t::RB},
	{"rc", argument_t::RC},
	{"rd", argument_t::RD},
};

static const argument get_arg(std::string token) {
	argument output;


	try {
		output.type = argument_map.at(token);
	} catch (std::out_of_range e) {
		// if we failed to find a match then it must be a constant (or error);
		output.type = argument_t::CONST;
	}

	if (output.type == argument_t::CONST) {
		try {
			output.value = std::stoi(token);
		} catch (std::logic_error e) {
			throw std::logic_error("Failed to parse token as register or constant: " + token);
		}

		if (output.value < 0 || output.value >= 256) {
			throw std::logic_error("Parsed constant token out of range (0 .. 255): " + output.value);
		}
	}

	return output;
}

static std::string to_string(argument_t t) {
	switch (t) {
	case argument_t::CONST:
		return "const";
	case argument_t::RA:
		return "ra";
	case argument_t::RB:
		return "rb";
	case argument_t::RC:
		return "rc";
	case argument_t::RD:
		return "rd";
	case argument_t::NONE:
		return "none";
	}

	// Should never hit this
	return "none";
}

static instruction_type token_to_instruction(const std::vector<std::string>& tokens) {
	instruction_type output;

	output.opcode = instruction_map.at(tokens.at(0));

	const instruction_info& i_info = instructions.at(output.opcode);

	for (int arg = 0; arg < NUMBER_ARGS; ++arg) {
		if (tokens.size() <= arg + 1) {
			// check that none is allowed
			bool good = false;
			for (const argument_t& allowed : i_info.allowable[arg]) {
				if (allowed == argument_t::NONE) {
					good = true;
				}
			}
			if (!good) {
				throw std::logic_error("Fewer than required arguments found.");
			}
			continue;
		}

		if (i_info.allowable[arg].size() == 0) {
			// got an arg, but none is allowed
			throw std::logic_error("Got more arguments then were expected");
		}

		argument a = get_arg(tokens.at(arg + 1));
		bool good = false;
		for (const argument_t& allowed : i_info.allowable[arg]) {
			if (a.type == allowed) {
				good = true;
			}
		}

		if (!good) {
			throw std::logic_error("Got an argument of type that wasn't expected. Got: " + to_string(a.type));
		}

		output.arg[arg] = a;
	}

	return output;
}

std::string machine_to_string(std::uint16_t machine) {
	std::stringstream output;
	output << std::hex << std::setfill('0') << std::setw(4) << machine;
	return output.str();
}

std::map<opcode, int> opcode_to_machine = 
{
	{opcode::ADD,  0},
	{opcode::SUB,  1},
	{opcode::AND,  2},
	{opcode::OR,   3},
	{opcode::XOR,  4},
	{opcode::SHFTL,5},
	{opcode::SHFTR,5},
	{opcode::LOAD, 6},
	{opcode::STORE,7},
	{opcode::MOV,  8},
	{opcode::JMP,  9},
	{opcode::JIFZ, 9},
	{opcode::JINZ, 9},
	{opcode::JGZ,  9},
	{opcode::LOADC,10},
	{opcode::OUT,  11},
	{opcode::HALT, 14},
	{opcode::NOP,  15},
};

std::string instruction_to_machine(instruction_type inst) {
	std::uint16_t machine = 0;

	// Top four bits are always opcode
	machine |= opcode_to_machine.at(inst.opcode) << 12;

	// Bottom 8 bits are always constant if it exists
	for (argument a : inst.arg) {
		if (a.type == argument_t::CONST) {
			machine += static_cast<std::uint16_t>(a.value);
		}
	}
	
	switch (inst.opcode) {
	case opcode::ADD:
	case opcode::SUB:
	case opcode::AND:
	case opcode::OR:
	case opcode::XOR:
	{
		int dest = 0;
		switch (inst.arg[0].type) {
		case argument_t::RA: dest = 0; break;
		case argument_t::RB: dest = 1; break;
		case argument_t::RC: dest = 2; break;
		case argument_t::RD: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		int source1 = 0;
		switch (inst.arg[1].type) {
		case argument_t::RA: source1 = 0; break;
		case argument_t::RC: source1 = 1; break;
		default:
			throw std::logic_error("Should never get here: invalid source1");
		}

		int source2 = 0;
		switch (inst.arg[2].type) {
		case argument_t::RB: source2 = 0; break;
		case argument_t::CONST: source2 = 1; break;
		default:
			throw std::logic_error("Should never get here: invalid source2");
		}

		machine |= dest << 10;
		machine |= source1 << 9;
		machine |= source2 << 8;

		break;
	}
	case opcode::SHFTL:
	case opcode::SHFTR:
	{
		int dest = 0;
		switch (inst.arg[0].type) {
		case argument_t::RA: dest = 0; break;
		case argument_t::RB: dest = 1; break;
		case argument_t::RC: dest = 2; break;
		case argument_t::RD: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		int source = 0;
		switch (inst.arg[1].type) {
		case argument_t::RB: source = 0; break;
		case argument_t::CONST: source = 1; break;
		default:
			throw std::logic_error("Should never get here: invalid source");
		}

		int direction = (inst.opcode == opcode::SHFTR) ? 0 : 1;

		machine |= dest << 10;
		machine |= source << 9;
		machine |= direction << 8;

		break;
	}
	case opcode::LOAD:
	case opcode::STORE:
	{
		int dest = 0;
		switch (inst.arg[0].type) {
		case argument_t::RA: dest = 0; break;
		case argument_t::RB: dest = 1; break;
		case argument_t::RC: dest = 2; break;
		case argument_t::RD: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		int offset = 0;
		switch (inst.arg[1].type) {
		case argument_t::RA:    offset = 0; break;
		case argument_t::RB:    offset = 1; break;
		case argument_t::RC:    offset = 2; break;
		case argument_t::CONST: offset = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid offset reg");
		}

		machine |= dest << 10;
		machine |= offset << 8;

		break;
	}
	case opcode::MOV:
	{
		int dest = 0;
		switch (inst.arg[0].type) {
		case argument_t::RA: dest = 0; break;
		case argument_t::RB: dest = 1; break;
		case argument_t::RC: dest = 2; break;
		case argument_t::RD: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		int source = 0;
		switch (inst.arg[1].type) {
		case argument_t::RA: source = 0; break;
		case argument_t::RB: source = 1; break;
		case argument_t::RC: source = 2; break;
		case argument_t::RD: source = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid source reg");
		}

		machine |= dest << 10;
		machine |= source << 8;

		break;
	}
	case opcode::JMP:
	case opcode::JIFZ:
	case opcode::JINZ:
	case opcode::JGZ:
	{
		int type_of_jmp = 0;
		switch (inst.opcode) {
		case opcode::JMP:  type_of_jmp = 0; break;
		case opcode::JIFZ: type_of_jmp = 1; break;
		case opcode::JINZ: type_of_jmp = 2; break;
		case opcode::JGZ:  type_of_jmp = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid jmp instruction");
		}

		int offset = 0;
		switch (inst.arg[1].type) {
		case argument_t::RA:    offset = 0; break;
		case argument_t::RB:    offset = 1; break;
		case argument_t::RC:    offset = 2; break;
		case argument_t::CONST: offset = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid offset reg");
		}

		machine |= type_of_jmp << 10;
		machine |= offset << 8;

		break;
	}
	case opcode::LOADC:
	{
		int dest = 0;
		switch (inst.arg[0].type) {
		case argument_t::RA: dest = 0; break;
		case argument_t::RB: dest = 1; break;
		case argument_t::RC: dest = 2; break;
		case argument_t::RD: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		machine |= dest << 10;
		break;
	}
	case opcode::LOADCH:
	{
		int dest = 0;
		switch (inst.arg[0].type) {
		case argument_t::RA: dest = 0; break;
		case argument_t::RB: dest = 1; break;
		case argument_t::RC: dest = 2; break;
		case argument_t::RD: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		machine |= dest << 10;
		machine |= 1 << 9; // this is what makes it the high part of reg
		
		break;
	}
	case opcode::OUT:
	{
		int dest = 0;
		switch (inst.arg[0].type) {
		case argument_t::RA: dest = 0; break;
		case argument_t::RB: dest = 1; break;
		case argument_t::RC: dest = 2; break;
		case argument_t::RD: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		machine |= dest << 10;

		break;
	}
	case opcode::HALT:
	case opcode::NOP:
	{
		// No arguments so nothing to fill
		break;
	}

	default:
		throw std::logic_error("Should never get here: uninplemented instruction");
	}

	return machine_to_string(machine);
}

std::string assemble(std::string assembly) {
	std::vector<line_t> lines;

	// Parse assembly into lines
	{
		std::stringstream asm_wrapper(assembly);
		line_t line;
		int line_number = 1;
		while (std::getline(asm_wrapper, line.line, '\n')) {
			line.line_number = line_number++;
			lines.push_back(line);
		}
	}

	// Handle each line
	std::ostringstream output;
	for (line_t& line : lines) {
		
		try {
			line.line = convert_line(line.line);

			// split into tokens
			{
				std::string temp;
				std::stringstream input(line.line);
				while (std::getline(input, temp, ' ')) {
					line.tokens.push_back(temp);
				}
			}

			// parse tokens
			line.inst = token_to_instruction(line.tokens);

			// output to text
			output << instruction_to_machine(line.inst) << " ";
		} catch (std::logic_error e) {
			std::cout << "Encountered error on line: " << line.line_number << std::endl;
			std::cout << "Error: " << e.what() << std::endl;
		}
	}

	return output.str();
}