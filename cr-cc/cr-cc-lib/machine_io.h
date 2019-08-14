#pragma once


#include <string>
#include <vector>


std::string machine_inst_to_unformatted(const std::vector<std::string>& machine_instructions);

std::string machine_inst_to_formated(const std::vector<std::string>& machine_instructions);