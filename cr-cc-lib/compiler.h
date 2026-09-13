#pragma once

#include "cmdline_parser.h"
#include "object_code.h"
#include "file_io.h"

Object::Object_Container compile_tu(std::string filename, FileReader f);
int compile(Compiler_Options opt);
