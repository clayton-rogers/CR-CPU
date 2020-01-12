#pragma once

#include "file_io.h"

#include <string>

std::string preprocess(const std::string& code, FileReader fr);

std::string strip_comments(const std::string& code);
