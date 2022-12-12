#include "file_io.h"

#include <fstream>
#include <iostream>
#include <streambuf>
#ifdef __GNUC__
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

static bool file_exists(std::string filename)
{
	std::ifstream file(filename);
	if (!file) {
		return false;
	} else {
		return true;
	}
}

std::string read_file(std::string filename)
{
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

void write_file(std::string filename, std::string data)
{
	std::ofstream file(filename);

	file << data;
}

std::vector<std::uint16_t> read_bin_file(std::string filename)
{
	std::ifstream file(filename, std::ios::binary);

	if (!file) {
		throw std::logic_error("Error opening file: " + filename);
	}

	std::vector<std::uint16_t> output;

	// Get file size in bytes, then allocate half as uin16_t
	const auto file_size_bytes = fs::file_size(filename);
	if ((file_size_bytes % 2) != 0) {
		throw std::logic_error("Error file not multiple of 2, is this a binary file?: " + filename);
	}
	output.resize(file_size_bytes / 2);

	file.read(reinterpret_cast<char*>(output.data()), file_size_bytes);

	return output;
}

void write_bin_file(std::string filename, std::vector<std::uint16_t> data)
{
	std::ofstream file(filename, std::ios::binary);
	file.write(reinterpret_cast<char*>(data.data()), data.size() * 2); // each element is 16 bits
}

void FileReader::add_directory(const std::string& directory)
{
	directories.push_back(directory);
}

std::string FileReader::include_file(std::string from_file, std::string filename, bool stdlib_only) const
{
	// Where include files are searched for:
	// - relative to input file
	// - at all include paths
	// - at stdlib

	if (!stdlib_only) {
		const auto tu_path = get_directory_name(from_file);
		const std::string full_filename = tu_path + "/" + filename;
		if (file_exists(full_filename)) {
			return read_file(full_filename);
		}
	}

	if (!stdlib_only) {
		for (const auto& directory : directories) {
			std::string full_filename = directory + "/" + filename;

			if (file_exists(full_filename)) {
				return read_file(full_filename);
			}
		}
	}

	{
		std::string full_filename = stdlib_path + "/" + filename;
		if (file_exists(full_filename)) {
			return read_file(full_filename);
		}
	}

	// Error if we haven't found the file yet.
	for (const auto& path : directories) {
		std::cout << "Looked in path:" << path << std::endl;
	}
	throw std::logic_error("Could not find file: " + filename);
}

std::vector<std::string> read_directory(std::string directory)
{
	std::vector<std::string> ret;

	for (const auto& entry : fs::directory_iterator(directory)) {
		ret.push_back(entry.path().string());
	}

	return ret;
}

std::string get_base_filename(std::string filename)
{
	auto last_slash = filename.find_last_of("/");
	auto last_period = filename.find_last_of(".");
	if (last_slash == std::string::npos) {
		return filename.substr(0, last_period);
	} else {
		return filename.substr(last_slash + 1, (last_period - last_slash - 1));
	}
}

std::string get_file_extension(std::string filename)
{
	return filename.substr(filename.find_last_of(".") + 1);
}

std::string get_directory_name(std::string filename)
{
	auto last_slash = filename.find_last_of("/");
	if (last_slash == std::string::npos) {
		return "./";
	} else {
		return filename.substr(0, last_slash + 1);
	}
}
