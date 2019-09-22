#pragma once

#include <cstdint>
#include <vector>
#include <string>


class Simulator {
public:

	Simulator(const std::vector<std::string>& input_instructions,
		int size_of_ram) :
		instructions(input_instructions)
	{
		ram.resize(size_of_ram, 0);
	}
	Simulator() = delete;

	void step();

	// **** PUBLIC DATA *** //
	std::uint16_t output = 0;
	std::uint16_t input = 0;
	std::uint16_t inst = 0;
	std::uint16_t next_inst = 0;
	std::uint16_t pc = 0;
	bool is_halted = false;

private:
	// **** DATA **** //
	std::uint16_t ra = 0, rb = 0, bp = 0, sp = 0;
	std::uint8_t addr = 0;

	int state = 0;

	std::vector<std::uint16_t> ram;
	const std::vector<std::string> instructions;


	// **** FUNCTIONS **** //
	std::uint16_t to_uint(const std::string& instruction);
	std::uint16_t& get_reg(int index);
};