#pragma once

#include "object_code.h"

#include <vector>

// Returns an "Object_Code", but more specifically, should always return one of type EXECUTABLE
Object::Object_Container link(std::vector<Object::Object_Container>&& objects);