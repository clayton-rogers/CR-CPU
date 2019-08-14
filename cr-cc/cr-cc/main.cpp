#include "assembler.h"
#include "file_reader.h"

#include <iostream>
#include <string>

int main(int argc, char **argv) {
	
	if (argc != 2) {
		std::cout << "Need file for input!!" << std::endl;
		return 1;
	}

	std::string filename(argv[1]);

	std::cout << "Filename: " << filename << std::endl;

	std::string file_contents = file_reader(filename);

	std::cout << "\nFile contents: \n" << file_contents << std::endl;

	std::string machine_code;
	try {
		machine_code = assemble(file_contents);
		std::cout << "\nMachine code: \n" << machine_code << std::endl;
	} catch (std::logic_error& e) {
		std::cout << "ASSEMBLY ERROR: \n";
		std::cout << e.what() << std::endl;
	}

	return 0;
}
