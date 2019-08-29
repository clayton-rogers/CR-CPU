#pragma once

#include <string>
#include <vector>

std::vector<std::string> assemble(const std::string& assembly);

// Returns true when the internal test passes
bool assembler_internal_test();