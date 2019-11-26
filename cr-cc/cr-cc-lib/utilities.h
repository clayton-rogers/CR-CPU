#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::uint16_t to_uint(const std::string& instruction);

std::string u16_to_string(std::uint16_t value);

std::string machine_inst_to_simple_hex(const std::vector<std::uint16_t>& machine_instructions);

std::string machine_inst_to_hex(const std::vector<std::uint16_t>& machine_instructions);

std::string machine_inst_to_srec(const std::vector<std::uint16_t>& machine_instructions, std::uint16_t address);
