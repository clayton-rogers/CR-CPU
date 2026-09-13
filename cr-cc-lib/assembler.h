#pragma once

#include "object_code.h"

#include <string>
#include <vector>
#include <cstdint>

Object::Object_Container assemble(const std::string& assembly);

// Returns true when the internal test passes
bool assembler_internal_test();
