#include "file_io.h"

#include <fstream>
#include <iostream>
#include <streambuf>

std::string read_file(std::string filename) {
	std::ifstream file(filename);

	if (!file) {
		throw std::logic_error("Error opening file: " + filename);
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

void write_file(std::string filename, std::string data) {
	std::ofstream file(filename, std::ios::binary);

	file << data;
}

std::vector<std::uint16_t> read_bin_file(std::string filename) {
	std::ifstream file(filename, std::ios::binary);

	std::vector<std::uint16_t> output;

	file.seekg(0, std::ios::end);
	// Get file size in bytes, then allocate half as uin16_t
	const unsigned int file_size_bytes = static_cast<unsigned int>(file.tellg());
	output.resize(file_size_bytes/2);

	file.seekg(0, std::ios::beg);

	file.read(reinterpret_cast<char*>(output.data()), file_size_bytes);

	return output;
}

void write_bin_file(std::string filename, std::vector<std::uint16_t> data) {
	std::ofstream file(filename, std::ios::binary);
	file.write(reinterpret_cast<char*>(data.data()), data.size() * 2); // each element is 16 bits
}

void FileReader::add_directory(const std::string& directory)
{
	directories.push_back(directory);
}

std::string FileReader::read_file_from_directories(std::string filename)
{
	std::string result;
	for (const auto& directory : directories) {
		try {
			result = read_file(directory + "/" + filename);
			break;
		} catch (std::logic_error e) {
			continue;
		}
	}

	return result;
}
