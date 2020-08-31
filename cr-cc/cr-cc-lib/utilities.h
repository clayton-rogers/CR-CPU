#pragma once

#include "object_code.h"

#include <cstdint>
#include <string>
#include <vector>

std::uint16_t string_to_u16(const std::string& instruction);

std::string u16_to_string(std::uint16_t value);

std::string machine_inst_to_simple_hex(const std::vector<std::uint16_t>& machine_instructions);

std::string machine_inst_to_hex(const std::vector<std::uint16_t>& machine_instructions);

std::string exe_to_srec(const Object::Object_Container& obj);
