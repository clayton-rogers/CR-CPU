#pragma once

#include <string>
#include <vector>
#include <cstdint>

std::string read_file(std::string filename);
void write_file(std::string filename, std::string data);

std::vector<std::uint16_t> read_bin_file(std::string filename);
void write_bin_file(std::string filename, std::vector<std::uint16_t> data);

std::vector<std::string> read_directory(std::string directory);

std::string get_base_filename(std::string filename);
std::string get_file_extension(std::string filename);

class FileReader {
public:
	void add_directory(const std::string& directory);
	std::string read_file_from_directories(std::string filename);
	std::vector<std::uint16_t> read_bin_file_from_directories(std::string filename);

private:
	std::vector<std::string> directories = { "./" };
};