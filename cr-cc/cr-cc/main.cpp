#include "assembler.h"

#include <iostream>
#include <string>
#include <fstream>
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

int main(int argc, char **argv) {
	
	if (argc != 2) {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	std::string filename(argv[1]);

	std::cout << "Filename: " << filename << std::endl;

	std::string file_contents = file_reader(filename);

	std::cout << "File contents: \n" << file_contents << std::endl;

	std::string machine_code = assemble(file_contents);

	std::cout << "Machine code: \n" << machine_code << std::endl;

	return 0;
}
