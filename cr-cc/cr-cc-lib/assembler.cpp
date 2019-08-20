#include "assembler.h"

#include <map>
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
	std::string label_value = "";
};

// one destination and up to two arguments
static const int NUMBER_ARGS = 3;
static const int CONST_LABEL_VALUE = -256;

struct instruction_t {
	opcode opcode = opcode::NOP;
	argument arg[NUMBER_ARGS];
};

struct line_t {
	int line_number;
	std::string line;
	std::vector<std::string> tokens;
	bool is_instruction;
	int instruction_number;
	instruction_t inst;
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

static const instruction_info NO_ARGS = { NO_ARG,NO_ARG,NO_ARG };

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

static std::string convert_line(const std::string& input) {
	std::string output;
	output.reserve(input.size());

	char last = ' ';
	for (int i = 0; i < input.size(); ++i) {
		char current = input.at(i);

		current = static_cast<char>(std::tolower(current));
		if (current == '#') {
			break;
		}
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

		if (token.at(0) == '.') {
			output.label_value = token.substr(1, std::string::npos);
			output.value = CONST_LABEL_VALUE;
		} else if (token.length() > 2 &&
			token.at(0) == '0' &&
			token.at(1) == 'x') {
			try {
				output.value = std::stoi(token, 0, 16);
			} catch (std::logic_error e) {
				throw std::logic_error("Failed to parse token as register or constant: " + token);
			}
			if (output.value < 0 || output.value > 0xFF) {
				throw std::logic_error("Parsed hex constant out of range (0x00 .. 0xFF): " + std::to_string(output.value));
			}
		} else {
			try {
				output.value = std::stoi(token);
			} catch (std::logic_error e) {
				throw std::logic_error("Failed to parse token as register or constant: " + token);
			}
			if (output.value < -128 || output.value > 127) {
				throw std::logic_error("Parsed dec constant out of range (-128 .. 127): " + std::to_string(output.value));
			}
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

static instruction_t tokens_to_instruction(const std::vector<std::string>& tokens) {

	if (tokens.size() > 1 + NUMBER_ARGS) {
		throw std::logic_error("Too many arguments");
	}

	instruction_t output;

	try {
		output.opcode = instruction_map.at(tokens.at(0));
	} catch (const std::logic_error&) {
		throw std::logic_error("Invalid opcode: " + tokens.at(0));
	}

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

		if (arg > 0 &&
			output.arg[arg - 1].type == argument_t::CONST &&
			a.type == argument_t::CONST) {
			throw std::logic_error("Got two constants in a row");
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
	std::string temp = output.str();
	for (auto& c : temp) c = static_cast<char>(std::toupper(c));
	return temp;
}

std::map<opcode, int> opcode_to_machine =
{
	{opcode::ADD,   0},
	{opcode::SUB,   1},
	{opcode::AND,   2},
	{opcode::OR,    3},
	{opcode::XOR,   4},
	{opcode::SHFTL, 5},
	{opcode::SHFTR, 5},
	{opcode::LOAD,  6},
	{opcode::STORE, 7},
	{opcode::MOV,   8},
	{opcode::JMP,   9},
	{opcode::JIFZ,  9},
	{opcode::JINZ,  9},
	{opcode::JGZ,   9},
	{opcode::LOADC, 10},
	{opcode::LOADCH,10},
	{opcode::OUT,   11},
	{opcode::HALT,  14},
	{opcode::NOP,   15},
};

std::string instruction_to_machine(const instruction_t& inst, const int instruction_number, const std::map<std::string, int>& labels) {
	std::uint16_t machine = 0;

	// Top four bits are always opcode
	machine |= opcode_to_machine.at(inst.opcode) << 12;

	// Bottom 8 bits are always constant if it exists
	for (argument a : inst.arg) {
		if (a.type == argument_t::CONST) {
			int const_value = 0;
			if (a.value != CONST_LABEL_VALUE) {
				const_value = a.value;
			} else {
				switch (inst.opcode) {
				case opcode::JMP:
				case opcode::JIFZ:
				case opcode::JINZ:
				case opcode::JGZ:
					// label instruction numbers should be relative to current instruction
					const_value = labels.at(a.label_value) - instruction_number;
					break;
				default:
				{
					// else must be a variable label
					std::string label_str;
					std::string offset_str;
					int offset = 0;
					const auto beg = a.label_value.find('[');
					const auto end = a.label_value.find(']');
					if (beg != std::string::npos) {
						label_str = a.label_value.substr(0, beg);
						offset_str = a.label_value.substr(beg + 1, end - beg - 1);
						offset = std::stoi(offset_str);
					} else {
						label_str = a.label_value;
					}

					const_value = labels.at(label_str) + offset;
				}
					break;
				}
				
			}
			machine += static_cast<std::uint8_t>(const_value);
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
		switch (inst.arg[0].type) {
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

std::vector<std::string> assemble(std::string assembly) {
	std::vector<line_t> lines;
	std::map<std::string, int> labels;
	int current_line = 0;

	std::vector<std::string> output_machine_code;

	try {

		// Parse assembly into lines
		{
			std::stringstream asm_wrapper(assembly);
			line_t line;
			line.is_instruction = false; // as a default
			int line_number = 1;
			while (std::getline(asm_wrapper, line.line, '\n')) {
				line.line_number = line_number++;
				lines.push_back(line);
			}
		}

		// Tokenize
		for (line_t& line : lines) {
			current_line = line.line_number;

			line.line = convert_line(line.line);

			std::string temp;
			std::stringstream input(line.line);
			while (std::getline(input, temp, ' ')) {
				line.tokens.push_back(temp);
			}
		}

		// Find all labels and decide which lines are instructions
		int instruction_number = 0;
		int var_offset = 0;
		for (line_t& line : lines) {
			current_line = line.line_number;

			if (line.tokens.size() != 0) {
				if (line.tokens.at(0).at(0) == '.') {
					// lines with a . are either goto labels when they have ":"
					// otherwise are compiler directives.
					auto end_of_label = line.tokens.at(0).find(':');
					if (std::string::npos != end_of_label) {
						std::string label = line.tokens.at(0).substr(1, (end_of_label - 1));
						if (labels.find(label) != labels.end()) {
							throw std::logic_error("Duplicate label found: " + label);
						}
						labels[label] = instruction_number;
					} else {
						std::string directive = line.tokens.at(0).substr(1, std::string::npos);
						if (directive == "var") {
							// format is
							// .VAR int my_var
							// .VAR int[10] my_var
							const std::string type = line.tokens.at(1);
							const std::string var_name = line.tokens.at(2);
							labels[var_name] = var_offset;
							
							const auto beg = type.find('[');
							const auto end = type.find(']');
							if (beg == std::string::npos) {
								var_offset++;
							} else {
								std::string var_size_str = type.substr(beg + 1, end - beg);
								int var_size = std::stoi(var_size_str);
								var_offset += var_size;
							}
							
						} else {
							throw std::logic_error("Unknown directive: " + directive);
						}
					}
				} else {
					line.is_instruction = true;
					line.instruction_number = instruction_number;
					instruction_number++;
				}
			}
		}

		// Parse each line
		for (line_t& line : lines) {
			current_line = line.line_number;

			if (line.is_instruction) {
				// parse tokens
				line.inst = tokens_to_instruction(line.tokens);

				// output to text
				output_machine_code.push_back(instruction_to_machine(line.inst, line.instruction_number, labels));
			}
		}

	} catch (const std::logic_error& e) {
		std::string msg = std::string("line ") + std::to_string(current_line) + ": " + e.what();
		throw std::logic_error(msg);
	}

	return output_machine_code;
}