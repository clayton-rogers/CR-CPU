#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <variant>


namespace Object {

	using Stream_Type = std::vector<std::uint16_t>;

	class Section {
	public:
		
		enum class Section_Type {
			EXTERNAL_REFERENCES,
			RELOCATION,
			EXPORTED_SYMBOLS,
			TEXT,
			OBJECT,
		};
		Section_Type section_type;

		virtual ~Section() = default;
		Section(Section_Type type) : section_type(type) {}
		Stream_Type to_stream() const;

	private:
		virtual Stream_Type child_to_stream() const = 0;
	};

	class External_References : public Section {
	public:
		enum class Reference_Type {
			HI_BYTE,
			LO_BYTE,
		};
		struct External_Reference {
			std::string name;
			Reference_Type type;
			std::vector<std::uint16_t> locations;
		};
		std::vector<External_Reference> references;

		External_References() : Section(Section_Type::EXTERNAL_REFERENCES) {}
	private:
		Stream_Type child_to_stream() const override;
	};

	class Relocations : public Section {
	public:
		enum class Relocation_Type {
			HI_BYTE,
			LO_BYTE,
		};
		struct Relocation {
			Relocation_Type type;
			std::uint16_t offset;
		};
		std::vector<Relocation> relocation_locations;

		Relocations() : Section(Section_Type::RELOCATION) {}
	private:
		Stream_Type child_to_stream() const override;
	};

	class Exported_Symbols : public Section {
	public:
		enum class Symbol_Type {
			FUNCTION,
			VARIABLE,
		};
		struct Exported_Symbol {
			std::string name;
			Symbol_Type type;
			std::uint16_t offset;
		};
		std::vector<Exported_Symbol> symbols;

		Exported_Symbols() : Section(Section_Type::EXPORTED_SYMBOLS) {}
	private:
		Stream_Type child_to_stream() const override;
	};

	class Machine_Code : public Section {
	public:
		std::vector<std::uint16_t> machine_code;

		Machine_Code() : Section(Section_Type::TEXT) {}
	private:
		Stream_Type child_to_stream() const override;
	};




	struct Object_Type {
		Exported_Symbols exports;
		External_References references;
		Relocations relocations;
		Machine_Code machine_code;
	};

	struct Library_Type {
		std::vector<Object_Type> objects;
	};

	struct Executable {
		Machine_Code machine_code;
	};

	struct Map {
		Exported_Symbols exports;
	};

	// TODO shared lib?

	class Object_Code {
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
