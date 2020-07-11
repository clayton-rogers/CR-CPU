#pragma once

#include "object_code.h"

#include <vector>

// Returns an "Object_Code", but more specifically, should always return one of type EXECUTABLE
Object::Object_Container link(std::vector<Object::Object_Container>&& objects, int link_addr);

// Wraps up multple objects into a library
Object::Object_Container make_lib(const std::vector<Object::Object_Container>& objects);

// Converts a executable to a map (i.e. strips everything except for the exports)
Object::Object_Container to_map(const Object::Object_Container& obj);
