#pragma once

#include "file_io.h"

#include <string>

std::string preprocess(const std::string& tu_filename, FileReader fr);

std::string strip_comments(const std::string& code);
