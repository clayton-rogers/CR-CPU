#include "file_reader.h"

#include <fstream>
#include <iostream>
#include <streambuf>

std::string file_reader(std::string filename) {
	std::ifstream file(filename);

	if (!file) {
		std::cout << "Error opening file: " << filename << std::endl;
		return "";
	}

	std::string output;
	file.seekg(0, std::ios::end);
	output.reserve(static_cast<unsigned int>(file.tellg()));
	file.seekg(0, std::ios::beg);

	// NOTE: requires extra parentheses to prevent most vexing parse
	output.assign(
		(std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	return output;
}