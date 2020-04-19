#include "linker.h"
#include "cast.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

static constexpr auto SECTION_HEADER_SIZE = 2;
static constexpr auto OC_HEADER_SIZE = 4;

TEST_CASE("Test section code", "[link]") {
	using namespace Object;

	SECTION("External_References") {
		auto temp = Section::create_section(Section::Section_Type::EXTERNAL_REFERENCES);
		auto ext = dynamic_cast<External_References&>(*temp);

		SECTION("Empty") {
			auto stream = ext.to_stream();
			CHECK(stream.size() == SECTION_HEADER_SIZE);
			CHECK(stream.at(0) == u16(Section::Section_Type::EXTERNAL_REFERENCES));
			CHECK(stream.at(1) == 0); // size of section
		}

		SECTION("Add an item") {
			External_References::External_Reference ref;
			ref.name = "add";
			ref.type = External_References::Reference_Type::HI_BYTE;
			ref.locations.push_back(56);
			ref.locations.push_back(5543);

			ext.references.push_back(ref);

			auto stream = ext.to_stream();
			CHECK(stream.size() == SECTION_HEADER_SIZE + 4 + 1 + 2);
			CHECK(stream.at(0) == u16(Section::Section_Type::EXTERNAL_REFERENCES));
			CHECK(stream.at(1) == 4 + 1 + 2);
			CHECK(stream.at(2) == u16('a'));
			CHECK(stream.at(3) == u16('d'));
			CHECK(stream.at(4) == u16('d'));
			CHECK(stream.at(5) == 0); // null terminator
			CHECK(stream.at(6) == u16(External_References::Reference_Type::HI_BYTE));
			CHECK(stream.at(7) == 56);
			CHECK(stream.at(8) == 5543);
		}

		SECTION("Add two items") {
			External_References::External_Reference ref;
			ref.name = "add";
			ref.type = External_References::Reference_Type::LO_BYTE;
			ref.locations.push_back(57);

			ext.references.push_back(ref);

			ref.name = "add";
			ref.type = External_References::Reference_Type::HI_BYTE;
			ref.locations.clear();
			ref.locations.push_back(56);

			ext.references.push_back(ref);

			auto stream = ext.to_stream();
			const int ref_size = 4 + 1 + 1; // name = 4, type = 1, locations = 1
			CHECK(stream.size() == SECTION_HEADER_SIZE + 2*ref_size);
			CHECK(stream.at(0) == u16(Section::Section_Type::EXTERNAL_REFERENCES));
			CHECK(stream.at(1) == 2*ref_size);

			CHECK(stream.at(2) == u16('a'));
			CHECK(stream.at(3) == u16('d'));
			CHECK(stream.at(4) == u16('d'));
			CHECK(stream.at(5) == 0); // null terminator
			CHECK(stream.at(6) == u16(External_References::Reference_Type::LO_BYTE));
			CHECK(stream.at(7) == 57);

			CHECK(stream.at(8) == u16('a'));
			CHECK(stream.at(9) == u16('d'));
			CHECK(stream.at(10) == u16('d'));
			CHECK(stream.at(11) == 0); // null terminator
			CHECK(stream.at(12) == u16(External_References::Reference_Type::HI_BYTE));
			CHECK(stream.at(13) == 56);
		}
	}

	SECTION("Relocation") {
		auto temp = Section::create_section(Section::Section_Type::RELOCATION);
		auto rel = dynamic_cast<Relocation&>(*temp);

		SECTION("Empty") {
			auto stream = rel.to_stream();
			CHECK(stream.size() == SECTION_HEADER_SIZE);
			CHECK(stream.at(0) == u16(Section::Section_Type::RELOCATION));
			CHECK(stream.at(1) == 0); // size of section
		}

		SECTION("Add some items") {
			rel.relocation_locations.push_back(432);
			rel.relocation_locations.push_back(11);
			rel.relocation_locations.push_back(62323);

			auto stream = rel.to_stream();

			CHECK(stream.size() == SECTION_HEADER_SIZE + 3);
			CHECK(stream.at(0) == u16(Section::Section_Type::RELOCATION));
			CHECK(stream.at(1) == 3);

			CHECK(stream.at(2) == 432);
			CHECK(stream.at(3) == 11);
			CHECK(stream.at(4) == 62323);
		}
	}

	SECTION("Exported Symbols") {
		auto temp = Section::create_section(Section::Section_Type::EXPORTED_SYMBOLS);
		auto syms = dynamic_cast<Exported_Symbols&>(*temp);

		SECTION("Empty") {
			auto stream = syms.to_stream();
			CHECK(stream.size() == SECTION_HEADER_SIZE);
			CHECK(stream.at(0) == u16(Section::Section_Type::EXPORTED_SYMBOLS));
			CHECK(stream.at(1) == 0); // size of section
		}

		SECTION("Add some items") {
			Exported_Symbols::Exported_Symbol symbol;

			symbol.name = "my_var";
			symbol.type = Exported_Symbols::Symbol_Type::VARIABLE;
			symbol.offset = 0x1234;
			syms.symbols.push_back(symbol);

			symbol.name = "teset";
			symbol.type = Exported_Symbols::Symbol_Type::FUNCTION;
			symbol.offset = 0x200;
			syms.symbols.push_back(symbol);

			auto stream = syms.to_stream();
			const int size_of_first_symb = 7 + 1 + 1;
			const int size_of_second_symb = 6 + 1 + 1;
			CHECK(stream.size() == SECTION_HEADER_SIZE + size_of_first_symb + size_of_second_symb);
			CHECK(stream.at(0) == u16(Section::Section_Type::EXPORTED_SYMBOLS));
			CHECK(stream.at(1) == size_of_first_symb + size_of_second_symb); // size of section

			CHECK(stream.at(2) == u16('m'));
			CHECK(stream.at(3) == u16('y'));
			CHECK(stream.at(4) == u16('_'));
			CHECK(stream.at(5) == u16('v'));
			CHECK(stream.at(6) == u16('a'));
			CHECK(stream.at(7) == u16('r'));
			CHECK(stream.at(8) == 0); // null terminator
			CHECK(stream.at(9) == u16(Exported_Symbols::Symbol_Type::VARIABLE));
			CHECK(stream.at(10) == 0x1234);

			CHECK(stream.at(11) == u16('t'));
			CHECK(stream.at(12) == u16('e'));
			CHECK(stream.at(13) == u16('s'));
			CHECK(stream.at(14) == u16('e'));
			CHECK(stream.at(15) == u16('t'));
			CHECK(stream.at(16) == 0); // null terminator
			CHECK(stream.at(17) == u16(Exported_Symbols::Symbol_Type::FUNCTION));
			CHECK(stream.at(18) == 0x200);
		}
	}

	SECTION("Machine Code") {
		auto temp = Section::create_section(Section::Section_Type::TEXT);
		auto text = dynamic_cast<Machine_Code&>(*temp);

		SECTION("Empty") {
			auto stream = text.to_stream();
			CHECK(stream.size() == SECTION_HEADER_SIZE);
			CHECK(stream.at(0) == u16(Section::Section_Type::TEXT));
			CHECK(stream.at(1) == 0); // size of section
		}

		SECTION("Add some items") {
			text.machine_code.push_back(432);
			text.machine_code.push_back(11);
			text.machine_code.push_back(62323);

			auto stream = text.to_stream();

			CHECK(stream.size() == SECTION_HEADER_SIZE + 3);
			CHECK(stream.at(0) == u16(Section::Section_Type::TEXT));
			CHECK(stream.at(1) == 3);

			CHECK(stream.at(2) == 432);
			CHECK(stream.at(3) == 11);
			CHECK(stream.at(4) == 62323);
		}
	}
}

TEST_CASE("Test object code", "[link]") {
	using namespace Object;

	Object_Code oc;

	SECTION("Empty object to stream") {
		oc.type = Object_Code::Object_Type::EXECUTABLE;

		auto stream = oc.to_object_code();

		CHECK(stream.size() == OC_HEADER_SIZE);
		CHECK(stream.at(0) == Object_Code::MAGIC);
		CHECK(stream.at(1) == 1); // version
		CHECK(stream.at(2) == u16(Object_Code::Object_Type::EXECUTABLE));
		CHECK(stream.at(3) == 0); // section size
	}

	SECTION("Object with two sections") {
		oc.type = Object_Code::Object_Type::OBJECT;

		oc.sections.emplace_back(Section::create_section(Section::Section_Type::TEXT));
		oc.sections.emplace_back(Section::create_section(Section::Section_Type::EXPORTED_SYMBOLS));

		{
			auto& text = dynamic_cast<Machine_Code&>(*oc.sections.at(0));

			text.machine_code.push_back(432);
			text.machine_code.push_back(11);
			text.machine_code.push_back(62323);
		}

		{
			auto& syms = dynamic_cast<Exported_Symbols&>(*oc.sections.at(1));

			Exported_Symbols::Exported_Symbol symbol;

			symbol.name = "my_var";
			symbol.type = Exported_Symbols::Symbol_Type::VARIABLE;
			symbol.offset = 0x1234;
			syms.symbols.push_back(symbol);

			symbol.name = "teset";
			symbol.type = Exported_Symbols::Symbol_Type::FUNCTION;
			symbol.offset = 0x200;
			syms.symbols.push_back(symbol);
		}

		auto stream = oc.to_object_code();

		const int size_of_text = 3;
		const int size_of_first_symb = 7 + 1 + 1;
		const int size_of_second_symb = 6 + 1 + 1;
		const int size_of_symb = size_of_first_symb + size_of_second_symb;

		CHECK(stream.size() == OC_HEADER_SIZE + 2 * SECTION_HEADER_SIZE + size_of_text + size_of_symb);
		CHECK(stream.at(0) == Object_Code::MAGIC);
		CHECK(stream.at(1) == 1); // version
		CHECK(stream.at(2) == u16(Object_Code::Object_Type::OBJECT));
		CHECK(stream.at(3) == 2 * SECTION_HEADER_SIZE + size_of_text + size_of_symb);

		CHECK(stream.at(4) == u16(Section::Section_Type::TEXT));
		CHECK(stream.at(5) == size_of_text);
		CHECK(stream.at(6) == 432);
		CHECK(stream.at(7) == 11);
		CHECK(stream.at(8) == 62323);

		CHECK(stream.at(9) == u16(Section::Section_Type::EXPORTED_SYMBOLS));
		CHECK(stream.at(10) == size_of_symb); // size of section
		CHECK(stream.at(11) == u16('m'));
		CHECK(stream.at(12) == u16('y'));
		CHECK(stream.at(13) == u16('_'));
		CHECK(stream.at(14) == u16('v'));
		CHECK(stream.at(15) == u16('a'));
		CHECK(stream.at(16) == u16('r'));
		CHECK(stream.at(17) == 0); // null terminator
		CHECK(stream.at(18) == u16(Exported_Symbols::Symbol_Type::VARIABLE));
		CHECK(stream.at(19) == 0x1234);
		CHECK(stream.at(20) == u16('t'));
		CHECK(stream.at(21) == u16('e'));
		CHECK(stream.at(22) == u16('s'));
		CHECK(stream.at(23) == u16('e'));
		CHECK(stream.at(24) == u16('t'));
		CHECK(stream.at(25) == 0); // null terminator
		CHECK(stream.at(26) == u16(Exported_Symbols::Symbol_Type::FUNCTION));
		CHECK(stream.at(27) == 0x200);
	}

	SECTION("Library that contains other objects") {
		oc.type = Object_Code::Object_Type::LIBRARY;

		// Add two sub objects into the library
		oc.sections.emplace_back(std::make_unique<Object_Code>());
		oc.sections.emplace_back(std::make_unique<Object_Code>());

		{
			auto obj = dynamic_cast<Object_Code*>(oc.sections.at(0).get());
			obj->type = Object_Code::Object_Type::OBJECT;

			auto extern_ref = std::make_unique<External_References>();
			External_References::External_Reference ref;
			ref.name = "add";
			ref.type = External_References::Reference_Type::HI_BYTE;
			ref.locations.push_back(0x210);

			extern_ref->references.push_back(ref);

			obj->sections.push_back(std::move(extern_ref));

			auto text = std::make_unique<Machine_Code>();
			text->machine_code.push_back(0xfa13);
			text->machine_code.push_back(0x0145);

			obj->sections.push_back(std::move(text));
		}

		{
			auto obj = dynamic_cast<Object_Code*>(oc.sections.at(1).get());
			obj->type = Object_Code::Object_Type::OBJECT;

			auto extern_ref = std::make_unique<External_References>();
			External_References::External_Reference ref;
			ref.name = "sub";
			ref.type = External_References::Reference_Type::LO_BYTE;
			ref.locations.push_back(0x255);
			ref.locations.push_back(0x410);

			extern_ref->references.push_back(ref);

			obj->sections.push_back(std::move(extern_ref));

			auto text = std::make_unique<Machine_Code>();
			text->machine_code.push_back(0x1234);
			text->machine_code.push_back(0x9843);

			obj->sections.push_back(std::move(text));
		}

		auto stream = oc.to_object_code();

		CHECK(stream.size() == 41);

		// =================================
		// library header:
		CHECK(stream.at(0) == Object_Code::MAGIC);
		CHECK(stream.at(1) == 1); // version
		CHECK(stream.at(2) == u16(Object_Code::Object_Type::LIBRARY));
		CHECK(stream.at(3) == 37);  // size of contents

		// =================================
		// Object 1 header
		CHECK(stream.at(4) == u16(Section::Section_Type::OBJECT));
		CHECK(stream.at(5) == 16); // size of object section

		CHECK(stream.at(6) == Object_Code::MAGIC);
		CHECK(stream.at(7) == 1); // version
		CHECK(stream.at(8) == u16(Object_Code::Object_Type::OBJECT));
		CHECK(stream.at(9) == 12);  // size of object contents

		// extern ref
		CHECK(stream.at(10) == u16(Section::Section_Type::EXTERNAL_REFERENCES));
		CHECK(stream.at(11) == 6); // size of refs

		CHECK(stream.at(12) == u16('a'));
		CHECK(stream.at(13) == u16('d'));
		CHECK(stream.at(14) == u16('d'));
		CHECK(stream.at(15) == 0);
		CHECK(stream.at(16) == u16(External_References::Reference_Type::HI_BYTE));
		CHECK(stream.at(17) == 0x210);

		// text
		CHECK(stream.at(18) == u16(Section::Section_Type::TEXT));
		CHECK(stream.at(19) == 2); // size of text

		CHECK(stream.at(20) == 0xfa13);
		CHECK(stream.at(21) == 0x0145);

		// =================================
		// Object 2 header
		CHECK(stream.at(22) == u16(Section::Section_Type::OBJECT));
		CHECK(stream.at(23) == 17); // size of object section

		CHECK(stream.at(24) == Object_Code::MAGIC);
		CHECK(stream.at(25) == 1); // version
		CHECK(stream.at(26) == u16(Object_Code::Object_Type::OBJECT));
		CHECK(stream.at(27) == 13);  // size of object contents

				// extern ref
		CHECK(stream.at(28) == u16(Section::Section_Type::EXTERNAL_REFERENCES));
		CHECK(stream.at(29) == 7); // size of refs

		CHECK(stream.at(30) == u16('s'));
		CHECK(stream.at(31) == u16('u'));
		CHECK(stream.at(32) == u16('b'));
		CHECK(stream.at(33) == 0);
		CHECK(stream.at(34) == u16(External_References::Reference_Type::LO_BYTE));
		CHECK(stream.at(35) == 0x255);
		CHECK(stream.at(36) == 0x410);

		// text
		CHECK(stream.at(37) == u16(Section::Section_Type::TEXT));
		CHECK(stream.at(38) == 2); // size of text

		CHECK(stream.at(39) == 0x1234);
		CHECK(stream.at(40) == 0x9843);
	}
}
