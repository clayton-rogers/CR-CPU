#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <variant>


namespace Object
{

	using Stream_Type = std::vector<std::uint16_t>;

	enum class HI_LO_TYPE {
		LO_BYTE,
		HI_BYTE,
	};

	enum class Symbol_Type {
		FUNCTION,
		VARIABLE,
		DATA,
	};

	struct External_Reference {
		std::string name;
		HI_LO_TYPE type = HI_LO_TYPE::LO_BYTE;
		std::vector<std::uint16_t> locations;
	};

	struct Relocation {
		HI_LO_TYPE type;
		std::uint16_t location;
		std::uint16_t new_offset;
		Relocation(HI_LO_TYPE type, std::uint16_t location, std::uint16_t offset):
			type(type), location(location), new_offset(offset)
		{}
		Relocation():
			type(HI_LO_TYPE::LO_BYTE), location(0), new_offset(0)
		{}
	};

	struct Exported_Symbol {
		std::string name;
		Symbol_Type type = Symbol_Type::FUNCTION;
		std::uint16_t offset = 0;
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
		std::uint16_t load_address = 0;
		std::vector<std::uint16_t> machine_code;

		// For Debug only:
		std::vector<Exported_Symbol> exported_symbols;
	};

	struct Map {
		std::vector<Exported_Symbol> exported_symbols;
	};

	// TODO shared lib?

	class Object_Container {
	public:
		// Static
		static const std::uint16_t MAGIC = 0x5243; // ascii "CR"
		static const std::uint16_t OBJECT_VERSION = 6;

		// Members
		std::variant<Object_Type, Library_Type, Executable, Map> contents;
		enum Variant_Type {
			OBJECT,
			LIBRARY,
			EXECUTABLE,
			MAP,
		};

		// Methods
		Stream_Type to_stream() const;
		static Object_Container from_stream(const Stream_Type& s);
	};




	// **********************************************************************
	// Operators
	// **********************************************************************
	bool operator==(const Object_Container& a, const Object_Container& b);
	bool operator==(const Object_Type& a, const Object_Type& b);
	bool operator==(const Library_Type& a, const Library_Type& b);
	bool operator==(const Executable& a, const Executable& b);
	bool operator==(const Map& a, const Map& b);
	bool operator==(const Relocation& a, const Relocation& b);
	bool operator==(const External_Reference& a, const External_Reference& b);
	bool operator==(const Exported_Symbol& a, const Exported_Symbol& b);

	std::string to_string(const Object_Container& obj);
}
