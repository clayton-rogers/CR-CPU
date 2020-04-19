#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace Object {
	// size in u16's =
	// (name length + 1) + num locations
	struct External_Reference {
		std::string name;
		std::vector<std::uint16_t> locations;
	};

	// size in u16's =
	// (name length + 1) + 2
	struct Exported_Symbol {
		enum class Type {
			FUNCTION,
			VARIABLE,
		};
		std::string name;
		std::uint16_t offset;
		Type type;
	};

	struct Object_Code {
		enum class Object_Type {
			OBJECT,
			LIBRARY,
			EXECUTABLE,
		};
		static const std::uint16_t MAGIC = 0x4352; // ascii "CR"
		Object_Type type = Object_Type::OBJECT;

		std::vector<Exported_Symbol> exported_symboles;
		std::vector<External_Reference> external_references;
		std::vector<std::uint16_t> machine_code;
	};

}
