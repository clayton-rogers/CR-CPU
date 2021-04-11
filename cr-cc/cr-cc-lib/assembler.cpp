#include "assembler.h"

#include "utilities.h"
#include "cast.h"

#include <map>
#include <set>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <string>
#include <iostream>
#include <cstdint>

using namespace Object;

enum class OPCODE {
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
	LOADI,
	PUSH,
	POP,
	CALL,
	RET,
	LOADA,
	HALT,
	NOP,

	LAST,
	FIRST = ADD,
};

static const std::map<std::string, OPCODE> instruction_str_map = {
	{"add", OPCODE::ADD},
	{"sub", OPCODE::SUB},
	{"and", OPCODE::AND},
	{"or", OPCODE::OR},
	{"xor", OPCODE::XOR},
	{"shftl", OPCODE::SHFTL},
	{"shftr", OPCODE::SHFTR},
	{"load", OPCODE::LOAD},
	{"store", OPCODE::STORE},
	{"mov", OPCODE::MOV},
	{"jmp", OPCODE::JMP},
	{"loadi", OPCODE::LOADI},
	{"push", OPCODE::PUSH},
	{"pop", OPCODE::POP},
	{"call", OPCODE::CALL},
	{"ret", OPCODE::RET},
	{"loada", OPCODE::LOADA},
	{"halt", OPCODE::HALT},
	{"nop", OPCODE::NOP},
};


enum class ARG_TYPE {
	RA,
	RB,
	RP,
	SP,
	CONST,
	NONE,
};

static const std::map<std::string, ARG_TYPE> argument_str_map = {
	{"ra", ARG_TYPE::RA},
	{"rb", ARG_TYPE::RB},
	{"rp", ARG_TYPE::RP},
	{"sp", ARG_TYPE::SP},
};

enum class FLAGS_TYPE {
	RELATIVE, // For jumps and calls
	IF_ZERO,  // For conditional jumps
	IF_NON_ZERO, // "
	IF_GREATER_EQUAL_ZERO, // "
	HIGH_BYTE, // For load immediate
	POINTER_REGISTER, // For load/store
	STACK_POINTER, // "
};

static const std::map<std::string, FLAGS_TYPE> flags_str_map = {
	{"r", FLAGS_TYPE::RELATIVE},
	{"z", FLAGS_TYPE::IF_ZERO},
	{"nz", FLAGS_TYPE::IF_NON_ZERO},
	{"ge", FLAGS_TYPE::IF_GREATER_EQUAL_ZERO},
	{"h", FLAGS_TYPE::HIGH_BYTE},
	{"rp", FLAGS_TYPE::POINTER_REGISTER},
	{"sp", FLAGS_TYPE::STACK_POINTER},
	{"gz", FLAGS_TYPE::IF_GREATER_EQUAL_ZERO}, // for legacy purpose only
};

struct Argument {
	ARG_TYPE type = ARG_TYPE::RA;
	bool const_is_label = false; // whether the constant value is a immediate or label
	int value = 0;
	std::string label_value = "";
};

struct Instruction {
	int number = 0;
	OPCODE opcode = OPCODE::NOP;
	std::vector<FLAGS_TYPE> flags;
	std::vector<Argument> args;
};

struct Instruction_Info {
	// one destination and up to two arguments
	std::vector<ARG_TYPE> allowable_arg1 = {};
	std::vector<ARG_TYPE> allowable_arg2 = {};
	std::vector<FLAGS_TYPE> allowable_flags = {};
};

static const std::vector<ARG_TYPE> ALL_REGISTER = { ARG_TYPE::RA, ARG_TYPE::RB, ARG_TYPE::RP, ARG_TYPE::SP };
static const std::vector<ARG_TYPE> REGULAR_REGISTER_OR_CONSTANT = { ARG_TYPE::RA, ARG_TYPE::RB, ARG_TYPE::RP, ARG_TYPE::CONST };
static const std::vector<ARG_TYPE> ADDRESS_OFFSET = { ARG_TYPE::RP, ARG_TYPE::SP, ARG_TYPE::CONST };
static const std::vector<ARG_TYPE> NO_ARG = { ARG_TYPE::NONE };

static const Instruction_Info STANDARD_ARITHMETIC = {
	ALL_REGISTER, // Destination/arg1
	REGULAR_REGISTER_OR_CONSTANT, // Source/arg2
	{} // No flags allowed
};

static std::map<OPCODE, Instruction_Info> instruction_definitions = {
	{OPCODE::ADD,  STANDARD_ARITHMETIC},
	{OPCODE::SUB,  STANDARD_ARITHMETIC},
	{OPCODE::AND,  STANDARD_ARITHMETIC},
	{OPCODE::OR,   STANDARD_ARITHMETIC},
	{OPCODE::XOR,  STANDARD_ARITHMETIC},
	{OPCODE::SHFTL, {ALL_REGISTER, {ARG_TYPE::RB, ARG_TYPE::CONST},{}}},
	{OPCODE::SHFTR, {ALL_REGISTER, {ARG_TYPE::RB, ARG_TYPE::CONST},{}}},
	{OPCODE::LOAD,  {ALL_REGISTER, ADDRESS_OFFSET, {FLAGS_TYPE::POINTER_REGISTER, FLAGS_TYPE::STACK_POINTER}}},
	{OPCODE::STORE, {ALL_REGISTER, ADDRESS_OFFSET, {FLAGS_TYPE::POINTER_REGISTER, FLAGS_TYPE::STACK_POINTER}}},
	{OPCODE::MOV,   {ALL_REGISTER, ALL_REGISTER, {}}},
	{OPCODE::JMP,   {{ARG_TYPE::CONST}, NO_ARG, {FLAGS_TYPE::RELATIVE, FLAGS_TYPE::IF_GREATER_EQUAL_ZERO, FLAGS_TYPE::IF_NON_ZERO, FLAGS_TYPE::IF_ZERO}}},
	{OPCODE::LOADI, {ALL_REGISTER, {ARG_TYPE::CONST}, {FLAGS_TYPE::HIGH_BYTE}}},
	{OPCODE::PUSH,  {ALL_REGISTER, NO_ARG, {}}},
	{OPCODE::POP,   {ALL_REGISTER, NO_ARG, {}}},
	{OPCODE::CALL,  {{ARG_TYPE::CONST}, NO_ARG, {FLAGS_TYPE::RELATIVE}}},
	{OPCODE::RET,   {NO_ARG, NO_ARG, {}}},
	{OPCODE::LOADA, {{ARG_TYPE::CONST}, NO_ARG, {}}},
	{OPCODE::HALT,  {NO_ARG, NO_ARG, {}}},
	{OPCODE::NOP,   {NO_ARG, NO_ARG, {}}},
};

struct Data_Label {
	int offset = 0;
	int size = 0;
	bool has_values = false;
	std::vector<int> values;
};

struct External_Label {
	std::vector<int> hi_references;
	std::vector<int> lo_references;
};

struct AssemblerState {
	std::map<std::string, Data_Label> data_label_map;
	std::map<std::string, int> text_label_map;
	std::map<std::string, int> const_map;
	std::map<std::string, External_Label> extern_label_map;
	std::vector<Instruction> instructions;
	int current_line = 0;

	int static_allocation_offset = 0;
	int next_inst_addr = 0;

	Object_Container out;
};

static std::vector<std::string> split_by_lines(const std::string& assembly) {
	std::vector<std::string> lines;
	std::stringstream asm_wrapper(assembly);
	std::string temp;
	while (std::getline(asm_wrapper, temp, '\n')) {
		lines.push_back(temp);
	}

	return lines;
}

// Generic function to see if a vector contains a particular value.
template <typename T>
bool vector_contains(const T item, const std::vector<T>& list) {
	bool found = false;
	for (const auto& item_of_list : list) {
		if (item == item_of_list) {
			found = true;
			break;
		}
	}

	return found;
}

static std::string to_string(ARG_TYPE t) {
	switch (t) {
	case ARG_TYPE::CONST:
		return "const";
	case ARG_TYPE::RA:
		return "ra";
	case ARG_TYPE::RB:
		return "rb";
	case ARG_TYPE::RP:
		return "rp";
	case ARG_TYPE::SP:
		return "sp";
	case ARG_TYPE::NONE:
		return "none";
	}

	// Should never hit this
	return "none-unknown";
}

static const Argument parse_token_for_arg(std::string token) {
	Argument output;

	if (argument_str_map.count(token) == 1) {
		output.type = argument_str_map.at(token);
		return output;
	} else {
		// if we failed to find a match then it must be a constant (or error);
		output.type = ARG_TYPE::CONST;
	}

	// If we got here then it must be a constant
	if (token.at(0) == '.') {
		output.const_is_label = true;
		output.label_value = token.substr(1, std::string::npos);
	} else if (token.length() > 2 && token.at(0) == '0' && token.at(1) == 'x') {
		try {
			output.value = std::stoi(token, 0, 16);
		} catch (const std::logic_error& /*e*/) {
			throw std::logic_error("Failed to parse token as register or constant: " + token);
		}
		if (output.value < 0 || output.value > 0xFF) {
			throw std::logic_error("Parsed hex constant out of range (0x00 .. 0xFF): " + std::to_string(output.value));
		}
	} else {
		try {
			output.value = std::stoi(token);
		} catch (const std::logic_error& /*e*/) {
			throw std::logic_error("Failed to parse token as register or constant: " + token);
		}
		if (output.value < -128 || output.value > 127) {
			throw std::logic_error("Parsed dec constant out of range (-128 .. 127): " + std::to_string(output.value));
		}
	}

	return output;
}

// This function makes lines more uniform for easier parsing. It:
// - converts to lowercase (except string literals)
// - strips comments ("#")
// - converts punctuation to space
// - strips extra space
// - converts each word into a token
static std::vector<std::string> tokenize_line(const std::string& input) {
	std::string converted;
	converted.reserve(input.size());

	// Perform transform
	char last = ' ';
	bool in_string = false;
	for (int i = 0; i < static_cast<int>(input.size()); ++i) {
		char current = input.at(i);

		if (!in_string) {
			current = static_cast<char>(std::tolower(current));
		}
		if (current == '#') {
			break;
		}
		if (current == ',' || current == '\t') {
			current = ' ';
		}

		if (current == ' ' && last == ' ') {
			// don't add
		} else {
			converted.append(&current, 1);
		}

		if (current == '"') {
			in_string = !in_string;
		}

		last = current;
	}

	// Separate into tokens
	std::vector<std::string> output;
	{
		std::string temp;
		std::stringstream converted_ss(converted);
		while (std::getline(converted_ss, temp, ' ')) {
			output.push_back(temp);
		}
	}

	return output;
}

static void handle_assembler_directive(const std::vector<std::string>& tokens, AssemblerState* as) {

	const std::string& directive = tokens.at(0);

	auto end_of_label = directive.find(':');
	if (end_of_label != std::string::npos) {
		// This is a goto label
		const std::string label = directive.substr(1, (end_of_label - 1));
		if (as->text_label_map.count(label) == 1) {
			throw std::logic_error("Duplicate label: " + label);
		}
		as->text_label_map[label] = as->next_inst_addr;

		return;
	}

	// Static data
	//   Format:
	// .static 2 var_name
	// # number is size in 16bit words
	// .static 4 var_name 0xff00 12 32 44
	// # optionally may be prefilled with data, otherwise it is zero
	if (directive == ".static") {
		const std::string label = tokens.at(2);
		if (as->data_label_map.count(label) == 1) {
			throw std::logic_error("Duplication variable: " + label);
		}

		Data_Label vl;
		vl.size = std::stoi(tokens.at(1), 0, 0);
		if (tokens.size() != 3) {
			// supplied values might be a list of numbers or a string
			if (tokens.at(3).at(0) == '"') {
				// this is a string
				std::string supplied_string;
				bool found_open_quote = false;
				bool found_close_quote = false;
				for (int i = 3; i < static_cast<int>(tokens.size()); ++i) {
					if (found_open_quote) {
						supplied_string += ' ';
					}
					std::string current_token = tokens.at(i);
					for (int j = 0; j < static_cast<int>(current_token.size()); ++j) {
						char c = current_token.at(j);
						if (!found_open_quote && c == '"') {
							found_open_quote = true;
							continue;
						}
						if (c == '"') {
							found_close_quote = true;
							break;
						}
						supplied_string += c;
					}
					if (found_close_quote) { break; }
				}

				if (!found_close_quote) {
					throw std::logic_error("Missing closing quote for var: " + label);
				}

				if (static_cast<int>(supplied_string.size()) != vl.size) {
					throw std::logic_error("Incorrect size string: " + label +
						" Expected: " + std::to_string(vl.size) +
						" Actual: " + std::to_string(supplied_string.size()));
				}

				vl.has_values = true;

				for (int i = 0; i < static_cast<int>(supplied_string.size()); ++i) {
					char c = supplied_string.at(i);
					vl.values.push_back(c);
				}

			} else {
				// this is a list of numbers
				if (tokens.size() != static_cast<size_t>(3) + vl.size) {
					throw std::logic_error(
						"Incorrect number of args to static var: " + label +
						" Expected: " + std::to_string(vl.size) +
						" Actual: " + std::to_string(tokens.size() - 3));
				}

				vl.has_values = true;

				for (int i = 0; i < vl.size; ++i) {
					const int value = std::stoi(tokens.at(static_cast<size_t>(i) + 3), 0, 0);
					if (value < -32768 || value > 65535) {
						throw std::logic_error("Constant out of range: " + label + " " + std::to_string(value));
					}
					vl.values.push_back(value);
				}
			}
		}
		vl.offset = as->static_allocation_offset;
		as->data_label_map[label] = vl;
		as->static_allocation_offset += vl.size;

		return;
	}

	// Named constants
	//   Format:
	// .constant 0xabcd my_const
	// # all constants are 16bits
	// # when used at the argument to an instruction, the appropriate byte is used
	// # ex: loadi ra, .my_const # gives cd
	// # and loadi.h ra, .my_const # gives ab
	if (directive == ".constant") {
		const std::string label = tokens.at(2);
		const int size = std::stoi(tokens.at(1), 0, 0);
		if (size < -32768 || size > 65535) {
			throw std::logic_error("Constant out of range: " + label + " " + std::to_string(size));
		}
		if (as->const_map.count(label) != 0) {
			throw std::logic_error("Duplicate constant defined: " + label);
		}
		as->const_map[label] = size;

		return;
	}

	// Extern label declarations
	//   Format:
	// .extern a_function
	// # a_function can now be used as a label in the rest of the program
	// # an external reference will be created in the object file, which
	// # will need to be resolved at link time.
	// loada .a_function
	// call .a_function
	// # this may also be used for extern funtions. The programmer must keep
	// # track of which is which
	if (directive == ".extern") {
		auto extern_label = tokens.at(1);
		if (as->extern_label_map.count(extern_label) == 1) {
			// Duplicate extern labels are allowed
		} else {
			as->extern_label_map[extern_label] = External_Label(); // default construct
		}

		return;
	}

	// Export label
	//   Format:
	// .my_fun:
	// .export my_fun
	// # my_fun will now be accessable in other compilation units.
	// # Specifically, this will cause an exported symbol to be generated
	// # which the link will match with external references in other units.
	// TODO this should also work with global variables
	if (directive == ".export") {
		const auto export_label = tokens.at(1);
		if (as->text_label_map.count(export_label) == 0) {
			throw std::logic_error("Tried to export unknown symbol: " + export_label);
		}
		const auto offset = u16(as->text_label_map.at(export_label));
		auto& exported_symbols = std::get<Object_Type>(as->out.contents).exported_symbols;
		exported_symbols.push_back(Object::Exported_Symbol{ export_label, Object::Symbol_Type::FUNCTION, offset });

		return;
	}

	// If we didn't hit any of the ifs, then the directive is unknown
	throw std::logic_error("Encountered unknown assembler directive: " + tokens.at(0));
}

static Instruction tokens_to_instruction(const std::vector<std::string>& tokens) {

	// one input1/destination, one input2/constant
	static const int MAX_NUMBER_ARGS = 2;

	if (tokens.size() > 1 + MAX_NUMBER_ARGS) {
		throw std::logic_error("Too many arguments");
	}

	Instruction output;

	// TODO this should be doable without the try catch block
	try {
		std::stringstream full_opcode(tokens.at(0));
		std::string temp;
		std::getline(full_opcode, temp, '.');
		output.opcode = instruction_str_map.at(temp);
		while (std::getline(full_opcode, temp, '.')) {
			output.flags.push_back(flags_str_map.at(temp));
		}
	} catch (const std::logic_error&) {
		throw std::logic_error("Invalid opcode or flag: " + tokens.at(0));
	}

	const Instruction_Info& i_info = instruction_definitions.at(output.opcode);

	// Verify previously decoded flags
	for (const auto& flag : output.flags) {
		if (!vector_contains(flag, i_info.allowable_flags)) {
			throw std::logic_error("Received unexpected flag: " + tokens.at(0));
		}
	}

	if (tokens.size() == 1) {
		if (!vector_contains(ARG_TYPE::NONE, i_info.allowable_arg1)) {
			throw std::logic_error("Fewer than required arguments found.");
		}

		return output;
	}

	// Must have at least the first arg if here.
	Argument arg = parse_token_for_arg(tokens.at(1));
	if (!vector_contains(arg.type, i_info.allowable_arg1)) {
		throw std::logic_error("Got an argument 1 of type that wasn't expected. Got: " + to_string(arg.type));
	}

	output.args.push_back(arg);

	if (tokens.size() == 2) {
		if (!vector_contains(ARG_TYPE::NONE, i_info.allowable_arg2)) {
			throw std::logic_error("Fewer than required arguments found.");
		}

		return output;
	}

	// Must have both arguments if here.
	arg = parse_token_for_arg(tokens.at(2));
	if (!vector_contains(arg.type, i_info.allowable_arg2)) {
		throw std::logic_error("Got an argument 2 of type that wasn't expected. Got: " + to_string(arg.type));
	}

	output.args.push_back(arg);

	return output;
}

static std::uint16_t instruction_to_machine(
	const Instruction& inst,
	const int instruction_number,
	AssemblerState* as)
{
	const std::map<std::string, Data_Label>& data_label_map = as->data_label_map;
	const std::map<std::string, int>& text_label_map = as->text_label_map;
	const std::map<std::string, int>& const_label_map = as->const_map;
	auto& extern_label_map = as->extern_label_map;
	auto& relocations = std::get<Object_Type>(as->out.contents).relocations;

	const std::map<OPCODE, int> opcode_to_machine =
	{
		{OPCODE::ADD,   0},
		{OPCODE::SUB,   1},
		{OPCODE::AND,   2},
		{OPCODE::OR,    3},
		{OPCODE::XOR,   4},
		{OPCODE::SHFTL, 5},
		{OPCODE::SHFTR, 5},
		{OPCODE::LOAD,  6},
		{OPCODE::STORE, 7},
		{OPCODE::MOV,   8},
		{OPCODE::JMP,   9},
		{OPCODE::LOADI, 10},
		{OPCODE::PUSH,  11},
		{OPCODE::POP,   11},
		{OPCODE::CALL,  12},
		{OPCODE::RET,   12},
		{OPCODE::LOADA, 13},
		{OPCODE::HALT,  14},
		{OPCODE::NOP,   15},
	};

	auto get_label = [&data_label_map, &text_label_map, &const_label_map, &extern_label_map](std::string label) -> int {
		if (data_label_map.count(label) == 1) {
			return data_label_map.at(label).offset;
		}

		if (text_label_map.count(label) == 1) {
			return text_label_map.at(label);
		}

		if (const_label_map.count(label) == 1) {
			return const_label_map.at(label);
		}

		if (extern_label_map.count(label) == 1) {
			return 0; // all extern references are just filled with 0x00. The linker will fill them out properly.
		}

		throw std::logic_error("Label not found: " + label);
	};

	// All labels need relocation if they are referred to in absolute terms. Except const values, which never change
	auto need_relocation = [&data_label_map, &text_label_map, &const_label_map, &extern_label_map](std::string label) -> bool {
		if (const_label_map.count(label) == 1) {
			return false;
		}
		if (extern_label_map.count(label) == 1) {
			return false; // no data, so no point in relocating
		}

		return true;
	};

	auto handle_external_ref_if_needed = [&extern_label_map, instruction_number](std::string label, HI_LO_TYPE type) {
		// If this isn't an extern label, then nothing to do
		if (extern_label_map.count(label) == 0) {
			return;
		}
		// If it is, record the location so when then extern reference is known,
		// it can be filled in
		if (type == HI_LO_TYPE::HI_BYTE) {
			extern_label_map.at(label).hi_references.push_back(instruction_number);
		} else {
			extern_label_map.at(label).lo_references.push_back(instruction_number);
		}
	};

	std::uint16_t machine = 0;

	// Top four bits are always opcode
	machine |= opcode_to_machine.at(inst.opcode) << 12;

	// Bottom 8 bits are always constant if it exists
	// (This is the gory-est code of the whole project. GOOD LUCK!)
	for (Argument a : inst.args) {
		if (a.type == ARG_TYPE::CONST) {
			int const_value = 0;
			if (!a.const_is_label) {
				const_value = a.value;
			} else {
				switch (inst.opcode) {
				case OPCODE::JMP:
				case OPCODE::CALL:
					// label instruction numbers should be relative to current instruction
					if (vector_contains(FLAGS_TYPE::RELATIVE, inst.flags)) {
						if (extern_label_map.count(a.label_value)) {
							throw std::logic_error("Should not reference an external symbol with relative flag: " + a.label_value);
						}
						const_value = get_label(a.label_value) - instruction_number;
						if (const_value > +127 || const_value < -128) {
							throw std::logic_error("Jmp or call to label too far away: " + a.label_value);
						}
					} else {
						if (need_relocation(a.label_value)) {
							// if relocation is required, then the target is stored in the relocation
							relocations.push_back(
								Relocation{
									HI_LO_TYPE::LO_BYTE,
									u16(instruction_number),
									u16(get_label(a.label_value))});
							const_value = 0;
						} else {
							const_value = get_label(a.label_value);
						}
						handle_external_ref_if_needed(a.label_value, HI_LO_TYPE::LO_BYTE);
					}
					const_value &= 0xFF;
					break;

					// LOADA
					// LOAD
					// STORE - constant or var label
					// ARITHMETIC - constant only
					// (Not sure if this is correct?)
					// (TODO spell out the case statement)
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
						offset = std::stoi(offset_str, 0, 0);
					} else {
						label_str = a.label_value;
					}

					const_value = get_label(label_str) + offset;
					if (inst.opcode == OPCODE::LOADA || vector_contains(FLAGS_TYPE::HIGH_BYTE, inst.flags)) {
						if (need_relocation(label_str)) {
							relocations.push_back(
								Relocation{
									HI_LO_TYPE::HI_BYTE,
									u16(instruction_number),
									u16(const_value)});
							const_value = 0;
						} else {
							const_value >>= 8;
						}
						handle_external_ref_if_needed(label_str, HI_LO_TYPE::HI_BYTE);
					} else {
						if (need_relocation(label_str)) {
							relocations.push_back(
								Relocation{
									HI_LO_TYPE::LO_BYTE,
									u16(instruction_number),
									u16(const_value)});
							const_value = 0;
						} else {
							const_value &= 0xFF;
						}
						handle_external_ref_if_needed(label_str, HI_LO_TYPE::LO_BYTE);
					}
				}
				break;
				}

			}
			if (const_value < 0) {
				switch (inst.opcode) {
				case OPCODE::ADD:
				case OPCODE::SUB:
				case OPCODE::AND:
				case OPCODE::OR:
				case OPCODE::XOR:
				case OPCODE::SHFTL:
				case OPCODE::SHFTR:
					throw std::logic_error("Arithmetic instruction cannot have negative constant");
				default:
					break; // nop
				}
			}
			machine += static_cast<std::uint8_t>(const_value);
		}
	}

	switch (inst.opcode) {
	case OPCODE::ADD:
	case OPCODE::SUB:
	case OPCODE::AND:
	case OPCODE::OR:
	case OPCODE::XOR:
	{
		int dest = 0;
		switch (inst.args[0].type) {
		case ARG_TYPE::RA: dest = 0; break;
		case ARG_TYPE::RB: dest = 1; break;
		case ARG_TYPE::RP: dest = 2; break;
		case ARG_TYPE::SP: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		int source = 0;
		switch (inst.args[1].type) {
		case ARG_TYPE::RA: source = 0; break;
		case ARG_TYPE::RB: source = 1; break;
		case ARG_TYPE::RP: source = 2; break;
		case ARG_TYPE::CONST: source = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid source1");
		}

		machine |= dest << 10;
		machine |= source << 8;

		break;
	}
	case OPCODE::SHFTL:
	case OPCODE::SHFTR:
	{
		int dest = 0;
		switch (inst.args[0].type) {
		case ARG_TYPE::RA: dest = 0; break;
		case ARG_TYPE::RB: dest = 1; break;
		case ARG_TYPE::RP: dest = 2; break;
		case ARG_TYPE::SP: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		int source = 0;
		switch (inst.args[1].type) {
		case ARG_TYPE::RB: source = 0; break;
		case ARG_TYPE::CONST: source = 1; break;
		default:
			throw std::logic_error("Should never get here: invalid source");
		}
		if (inst.args[1].type == ARG_TYPE::CONST &&
			(machine & 0xFF) > 0x000F) {
			throw std::logic_error("Constant for shift should be 0 .. 15");
		}

		int direction = (inst.opcode == OPCODE::SHFTR) ? 0 : 1;

		machine |= dest << 10;
		machine |= source << 9;
		machine |= direction << 8;

		break;
	}
	case OPCODE::LOAD:
	case OPCODE::STORE:
	{
		int dest = 0;
		switch (inst.args[0].type) {
		case ARG_TYPE::RA: dest = 0; break;
		case ARG_TYPE::RB: dest = 1; break;
		case ARG_TYPE::RP: dest = 2; break;
		case ARG_TYPE::SP: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		// NOTE: 3 is never used
		int offset = 2; // by default relative to the address register
		if (vector_contains(FLAGS_TYPE::POINTER_REGISTER, inst.flags)) {
			offset = 0;
		}
		if (vector_contains(FLAGS_TYPE::STACK_POINTER, inst.flags)) {
			offset = 1;
		}
		if (vector_contains(FLAGS_TYPE::POINTER_REGISTER, inst.flags) &&
			vector_contains(FLAGS_TYPE::STACK_POINTER, inst.flags)) {
			throw std::logic_error("Cannot have both rp and sp offset flags");
		}

		machine |= dest << 10;
		machine |= offset << 8;

		break;
	}
	case OPCODE::MOV:
	{
		int dest = 0;
		switch (inst.args[0].type) {
		case ARG_TYPE::RA: dest = 0; break;
		case ARG_TYPE::RB: dest = 1; break;
		case ARG_TYPE::RP: dest = 2; break;
		case ARG_TYPE::SP: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		int source = 0;
		switch (inst.args[1].type) {
		case ARG_TYPE::RA: source = 0; break;
		case ARG_TYPE::RB: source = 1; break;
		case ARG_TYPE::RP: source = 2; break;
		case ARG_TYPE::SP: source = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid source reg");
		}

		machine |= dest << 10;
		machine |= source << 8;

		break;
	}
	case OPCODE::JMP:
	{
		int type_of_jmp = 0;
		if (vector_contains(FLAGS_TYPE::IF_ZERO, inst.flags)) {
			type_of_jmp = 1;
		}
		if (vector_contains(FLAGS_TYPE::IF_NON_ZERO, inst.flags)) {
			type_of_jmp = 2;
		}
		if (vector_contains(FLAGS_TYPE::IF_GREATER_EQUAL_ZERO, inst.flags)) {
			type_of_jmp = 3;
		}

		int offset = 0;
		if (vector_contains(FLAGS_TYPE::RELATIVE, inst.flags)) {
			offset = 0;

			if (inst.flags.size() > 2) {
				throw std::logic_error("Too many jump condition flags specified");
			}
		} else {
			offset = 1;

			if (inst.flags.size() > 1) {
				throw std::logic_error("Too many jump condition flags specified");
			}
		}

		machine |= type_of_jmp << 10;
		machine |= offset << 9;

		break;
	}
	case OPCODE::LOADI:
	{
		int dest = 0;
		switch (inst.args[0].type) {
		case ARG_TYPE::RA: dest = 0; break;
		case ARG_TYPE::RB: dest = 1; break;
		case ARG_TYPE::RP: dest = 2; break;
		case ARG_TYPE::SP: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		int high_flag = (vector_contains(FLAGS_TYPE::HIGH_BYTE, inst.flags)) ? 1 : 0;

		machine |= dest << 10;
		machine |= high_flag << 9;
		break;
	}
	case OPCODE::PUSH:
	case OPCODE::POP:
	{
		int dest = 0;
		switch (inst.args[0].type) {
		case ARG_TYPE::RA: dest = 0; break;
		case ARG_TYPE::RB: dest = 1; break;
		case ARG_TYPE::RP: dest = 2; break;
		case ARG_TYPE::SP: dest = 3; break;
		default:
			throw std::logic_error("Should never get here: invalid dest reg");
		}

		int type_of_operation = 0;
		switch (inst.opcode) {
		case OPCODE::PUSH: type_of_operation = 0; break;
		case OPCODE::POP: type_of_operation = 1; break;
		default:
			throw std::logic_error("Should never get here: invalid opcode push,pop");
		}

		machine |= dest << 10;
		machine |= type_of_operation << 8;

		break;
	}
	case OPCODE::CALL:
	case OPCODE::RET:
	{
		int type_of_operation = 0;
		switch (inst.opcode) {
		case OPCODE::CALL:
			type_of_operation = vector_contains(FLAGS_TYPE::RELATIVE, inst.flags) ? 0 : 2;
			break;
		case OPCODE::RET:
			type_of_operation = 1;
			break;
		default:
			throw std::logic_error("Should never get here: invalid call/ret type");
		}

		machine |= type_of_operation << 8;

		break;
	}
	case OPCODE::LOADA:
	case OPCODE::HALT:
	case OPCODE::NOP:
	{
		// No arguments so nothing to fill
		break;
	}

	default:
		throw std::logic_error("Should never get here: uninplemented instruction");
	}

	return machine;
}

static std::vector<std::uint16_t> generate_machine_code(AssemblerState* as) {
	auto& machine_code = std::get<Object_Type>(as->out.contents).machine_code;

	// Compile instructions
	for (const auto& inst : as->instructions) {
		machine_code.push_back(
			instruction_to_machine(inst, inst.number, as)
		);
	}

	// Get size of data section
	const int size_of_data = [&as]() -> int {
		int size = 0;
		for (const auto& label : as->data_label_map) {
			size += label.second.size;
		}
		return size;
	}();

	// Add contents of data section
	const auto start_of_static_data = machine_code.size();
	machine_code.resize(machine_code.size() + size_of_data);
	for (const auto& var : as->data_label_map) {
		Data_Label label = var.second;

		for (int i = 0; i < label.size; ++i) {
			int value = 0;
			if (label.has_values) {
				value = label.values.at(i);
			}
			machine_code.at(static_cast<size_t>(label.offset) + i) = static_cast<uint16_t>(value);
		}
	}

	// Add symbol for start of static data
	auto& exported_symbols = std::get<Object_Type>(as->out.contents).exported_symbols;
	if (size_of_data != 0) {
		Exported_Symbol sym;
		sym.name = "__static_data";
		sym.offset = static_cast<std::uint16_t>(start_of_static_data);
		sym.type = Symbol_Type::DATA;
		exported_symbols.push_back(sym);
	}

	// Add all external references to the object
	auto& extern_ref = std::get<Object_Type>(as->out.contents).external_references;
	for (const auto& ref : as->extern_label_map) {
		// If this extern is not actually referenced, then don't include it
		if (ref.second.hi_references.size() == 0 && ref.second.lo_references.size() == 0) {
			continue;
		}

		External_Reference hi_ref;
		External_Reference lo_ref;

		hi_ref.name = ref.first;
		lo_ref.name = ref.first;
		hi_ref.type = HI_LO_TYPE::HI_BYTE;
		lo_ref.type = HI_LO_TYPE::LO_BYTE;

		for (const auto& hi_location : ref.second.hi_references) {
			hi_ref.locations.push_back(u16(hi_location));
		}

		for (const auto& lo_location : ref.second.lo_references) {
			lo_ref.locations.push_back(u16(lo_location));
		}

		extern_ref.push_back(hi_ref);
		extern_ref.push_back(lo_ref);
	}

	return machine_code;
}

Object_Container assemble(const std::string& assembly) {

	const std::vector<std::string> lines = split_by_lines(assembly);

	AssemblerState as;

	try {

		for (const auto& line : lines) {
			++as.current_line;

			const std::vector<std::string> tokens = tokenize_line(line);

			// If this is an empty line
			if (tokens.size() == 0) {
				continue;
			}

			// If this is a compiler directive
			if (tokens.at(0).at(0) == '.') {
				handle_assembler_directive(tokens, &as);
			} else {
				// This is a code generating line
				Instruction instruction = tokens_to_instruction(tokens);
				instruction.number = as.next_inst_addr;
				++as.next_inst_addr;
				as.instructions.push_back(instruction);
			}
		}

	} catch (const std::logic_error& e) {
		std::string msg = std::string("line ") + std::to_string(as.current_line) + ": " + e.what();
		throw std::logic_error(msg);
	}

	// Data section is placed after the text section
	const int size_of_text = static_cast<int>(as.instructions.size());
	for (auto& label : as.data_label_map) {
		label.second.offset += size_of_text;
	}

	generate_machine_code(&as);
	return as.out;
}

bool assembler_internal_test() {

	int num_opcodes = 0;

	// Test that every opcode has a definition
	for (int i = static_cast<int>(OPCODE::FIRST); i < static_cast<int>(OPCODE::LAST); ++i) {
		OPCODE op = static_cast<OPCODE>(i);
		++num_opcodes;

		if (instruction_definitions.find(op) == instruction_definitions.end()) {
			// Entry was not found, there is no definition for this opcode
			std::cout << "Could not find opcode # " << i << std::endl;
			return false;
		}
	}

	// check opcode strings
	{
		int size = static_cast<int>(instruction_str_map.size());
		if (size - 1 != static_cast<int>(OPCODE::NOP)) {
			std::cout << "Instruction map size wrong" << std::endl;
			return false;
		}
	}

	// check arg str map
	{
		int size = static_cast<int>(argument_str_map.size());
		if (size - 1 != static_cast<int>(ARG_TYPE::SP)) {
			std::cout << "Argument map size wrong" << std::endl;
			return false;
		}
	}

	// check flags str map
	{
		int size = static_cast<int>(flags_str_map.size()) - 1; // minus 1 for "gz" which is only for legacy
		if (size - 1 != static_cast<int>(FLAGS_TYPE::STACK_POINTER)) {
			std::cout << "Flags map size wrong" << std::endl;
			return false;
		}
	}


	// Return true if all the tests pass
	return true;
}
