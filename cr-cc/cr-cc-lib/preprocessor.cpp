#include "preprocessor.h"

#include <iostream>

std::string preprocess(const std::string& code, FileReader fr) {
	std::string preprocessed(code);

	while (true) {
		auto hash_offset = preprocessed.find('#');
		if (hash_offset == std::string::npos) {
			break;
		}

		auto nl_offset = preprocessed.find('\n', hash_offset);

		// +1 is to not include the # which will always be there
		std::string directive_line = preprocessed.substr(hash_offset + 1, nl_offset - hash_offset);


		// Delete the directive
		preprocessed.erase(hash_offset, nl_offset - hash_offset);

		std::string directive = directive_line.substr(0, directive_line.find(' '));

		if (false) { // To align the rest of the cases
		} else if ("include" == directive) {
			const auto first = directive_line.find('"');
			const auto second = directive_line.find('"', first + 1);
			const std::string include_filename = directive_line.substr(first + 1, second - first - 1);
			const std::string include_text = fr.read_file_from_directories(include_filename);

			preprocessed.insert(hash_offset, include_text);
		} else if ("define" == directive) {
			std::cout << "Warning: define directive not currently supported" << std::endl;
		} else if ("pragma" == directive) {
			std::cout << "Warning: no pragma directives are supported" << std::endl;
		} else {
			throw std::logic_error("No support for compiler directive: " + directive);
		}
	}

	return preprocessed;
}

std::string strip_comments(const std::string& code) {
	std::string stripped(code);

	// strip all block comments
	while (true) {
		const auto comment_start = stripped.find("/*");
		if (comment_start == std::string::npos) {
			break;
		}
		const auto comment_end = stripped.find("*/");
		if (comment_end == std::string::npos) {
			throw std::logic_error("Could not find matching end block comment");
		}

		// +2 for the size of the end "*/"
		stripped.erase(comment_start, comment_end - comment_start + 2);
	}

	// strip all line comments
	while (true) {
		const auto comment_start = stripped.find("//");
		if (comment_start == std::string::npos) {
			break;
		}
		auto comment_end = stripped.find("\n", comment_start + 1);
		if (comment_end == std::string::npos) {
			// The must be no more newlines in the file, so delete till end of file
			comment_end = stripped.length();
		}

		// +1 for the NL
		stripped.erase(comment_start, comment_end - comment_start + 1);
	}

	return stripped;
}