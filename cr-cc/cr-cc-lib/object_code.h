#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <variant>


namespace Object {

	using Stream_Type = std::vector<std::uint16_t>;

	enum class HI_LO_TYPE {
		HI_BYTE,
		LO_BYTE,
	};

	enum class Symbol_Type {
		FUNCTION,
		VARIABLE,
	};

	struct External_Reference {
		std::string name;
		HI_LO_TYPE type;
		std::vector<std::uint16_t> locations;
	};

	struct Relocation {
		HI_LO_TYPE type;
		std::uint16_t offset;
	};

	struct Exported_Symbol {
		std::string name;
		Symbol_Type type;
		std::uint16_t offset;
	};

	struct Object_Type {
		std::vector<Exported_Symbol>    exported_symbols;
		std::vector<External_Reference> external_references;
		std::vector<Relocation>         relocations;
		std::vector<std::uint16_t>      machine_code;
	};

	struct Library_Type {
		std::vector<Object_Type> objects;
	};

	struct Executable {
		std::vector<std::uint16_t> machine_code;
	};

	struct Map {
		std::vector<Exported_Symbol> exported_symbols;
	};

	// TODO shared lib?

	class Object_Container {
	public:

		static const std::uint16_t MAGIC = 0x4352; // ascii "CR"
		std::uint16_t load_address = 0;

		std::variant<Object_Type, Library_Type, Executable, Map> contents;
		enum Variant_Type {
			OBJECT,
			LIBRARY,
			EXECUTABLE,
			MAP,
		};
		
		Stream_Type to_object_code() const;

	private:
		std::uint16_t object_version = 2;
	};
}
