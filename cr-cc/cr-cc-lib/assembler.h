#pragma once

#include <string>
#include <vector>
#include <cstdint>

std::vector<std::uint16_t> assemble(const std::string& assembly);

// Returns true when the internal test passes
bool assembler_internal_test();