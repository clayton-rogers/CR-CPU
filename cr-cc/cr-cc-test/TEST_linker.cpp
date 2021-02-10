#include "linker.h"
#include "cast.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

static constexpr auto SECTION_HEADER_SIZE = 2;
static constexpr auto OC_HEADER_SIZE = 4;

TEST_CASE("Test linker", "[link]") {
	using namespace Object;

	SECTION("Relocations") {
		Object_Container item1;
		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa00);
			obj.machine_code.push_back(0x1011);
			obj.machine_code.push_back(0xabcd);

			obj.relocations.emplace_back(
				Relocation{HI_LO_TYPE::HI_BYTE, 0, 0x1200} );

			item1.contents = obj;
		}

		Object_Container item2;
		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0x1011);

			obj.relocations.emplace_back(
				Relocation{ HI_LO_TYPE::HI_BYTE, 1, 0x1100 });
			obj.relocations.emplace_back(
				Relocation{ HI_LO_TYPE::LO_BYTE, 2, 0x0011 });

			obj.exported_symbols.emplace_back(
				Exported_Symbol{ "fun", Symbol_Type::FUNCTION, 0x03 });

			item2.contents = obj;
		}

		std::vector<Object_Container> items;
		items.push_back(item1);
		items.push_back(item2);

		static const std::uint16_t DEFAULT_LOAD_ADDR = 0x200;
		auto output = link(std::move(items), DEFAULT_LOAD_ADDR);

		CHECK(output.contents.index() == Object_Container::EXECUTABLE);
		CHECK(std::get<Executable>(output.contents).load_address == DEFAULT_LOAD_ADDR);
		
		const auto& debug_external_ref = std::get<Executable>(output.contents).exported_symbols;
		CHECK(debug_external_ref.at(0).name == "fun");
		CHECK(debug_external_ref.at(0).type == Symbol_Type::FUNCTION);
		CHECK(debug_external_ref.at(0).offset == 0x0206);

		// offset to apply is 0x200 + 0x03 = 0x203
		const auto& code = std::get<Executable>(output.contents).machine_code;
		CHECK(code.at(0) == 0xfa14);
		CHECK(code.at(1) == 0x1011);
		CHECK(code.at(2) == 0xabcd);
		CHECK(code.at(3) == 0xfa12);
		CHECK(code.at(4) == 0x1013);
		CHECK(code.at(5) == 0x1014);
		CHECK(code.at(6) == 0x1011);
	}

	SECTION("External references and exported symbols") {
		std::vector<Object_Container> items(2);

		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0xabcd);
			obj.machine_code.push_back(0xac00);
			obj.machine_code.push_back(0xad00);

			obj.exported_symbols.push_back(
				Exported_Symbol{ "funa", Symbol_Type::FUNCTION, 0x01 }
			);

			obj.external_references.push_back(
				External_Reference{ "funb", HI_LO_TYPE::LO_BYTE, {0x01, 0x03} }
			);
			obj.external_references.push_back(
				External_Reference{ "funb", HI_LO_TYPE::HI_BYTE, {0x04} }
			);

			items.at(0).contents = obj;
		}

		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0x1011);

			obj.exported_symbols.push_back(
				Exported_Symbol{ "funb", Symbol_Type::FUNCTION, 0x03 }
			);

			obj.external_references.push_back(
				External_Reference{ "funa", HI_LO_TYPE::LO_BYTE, {0x01} }
			);
			obj.external_references.push_back(
				External_Reference{ "funa", HI_LO_TYPE::HI_BYTE, {0x02} }
			);


			items.at(1).contents = obj;
		}

		static const std::uint16_t DEFAULT_LOAD_ADDR = 0x200;
		auto output = link(std::move(items), DEFAULT_LOAD_ADDR);

		CHECK(output.contents.index() == Object_Container::EXECUTABLE);
		CHECK(std::get<Executable>(output.contents).load_address == DEFAULT_LOAD_ADDR);

		const auto& debug_external_ref = std::get<Executable>(output.contents).exported_symbols;
		CHECK(debug_external_ref.at(0).name == "funa");
		CHECK(debug_external_ref.at(0).type == Symbol_Type::FUNCTION);
		CHECK(debug_external_ref.at(0).offset == 0x0201);
		CHECK(debug_external_ref.at(1).name == "funb");
		CHECK(debug_external_ref.at(1).type == Symbol_Type::FUNCTION);
		CHECK(debug_external_ref.at(1).offset == 0x0208);

		// offset to apply is 0x200 + 0x03 = 0x203
		const auto& code = std::get<Executable>(output.contents).machine_code;
		CHECK(code.at(0) == 0xfa12);
		CHECK(code.at(1) == 0x1008);
		CHECK(code.at(2) == 0xabcd);
		CHECK(code.at(3) == 0xac08);
		CHECK(code.at(4) == 0xad02);
		CHECK(code.at(5) == 0xfa12);
		CHECK(code.at(6) == 0x1001);
		CHECK(code.at(7) == 0x1002);
		CHECK(code.at(8) == 0x1011);
	}
}

TEST_CASE("Test library creation", "[link]") {
	using namespace Object;

	SECTION("Test that libraries can be created") {
		Object_Container item1;
		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa00);
			obj.machine_code.push_back(0x1011);
			obj.machine_code.push_back(0xabcd);

			obj.relocations.emplace_back(
				Relocation{ HI_LO_TYPE::HI_BYTE, 0, 0x1200});

			item1.contents = obj;
		}

		Object_Container item2;
		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x4500);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0x1011);

			obj.relocations.emplace_back(
				Relocation{ HI_LO_TYPE::HI_BYTE, 1, 0x6100 });
			obj.relocations.emplace_back(
				Relocation{ HI_LO_TYPE::LO_BYTE, 2, 0x1100 });

			obj.exported_symbols.emplace_back(
				Exported_Symbol{ "fun", Symbol_Type::FUNCTION, 0x03 });

			item2.contents = obj;
		}

		Object_Container item3;
		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x4787);
			obj.machine_code.push_back(0x1241);
			obj.machine_code.push_back(0x7893);

			obj.relocations.emplace_back(
				Relocation{ HI_LO_TYPE::HI_BYTE, 123, 0x0011 }); // doesn't matter
			obj.relocations.emplace_back(
				Relocation{ HI_LO_TYPE::LO_BYTE, 11, 0x0033 }); // doesn't matter

			obj.exported_symbols.emplace_back(
				Exported_Symbol{ "fun2", Symbol_Type::FUNCTION, 0x0241 });
			obj.exported_symbols.emplace_back(
				Exported_Symbol{ "fun3", Symbol_Type::FUNCTION, 0x0244 });

			item3.contents = obj;
		}

		std::vector<Object_Container> items;
		items.push_back(item1);
		items.push_back(item2);
		items.push_back(item3);

		auto lib = make_lib(items);

		const auto& lib_contents = std::get<Library_Type>(lib.contents);
		const auto& item1_contents = std::get<Object_Type>(item1.contents);
		const auto& item2_contents = std::get<Object_Type>(item2.contents);
		const auto& item3_contents = std::get<Object_Type>(item3.contents);

		CHECK(lib_contents.objects.at(0) == item1_contents);
		CHECK(lib_contents.objects.at(1) == item2_contents);
		CHECK(lib_contents.objects.at(2) == item3_contents);
	}

	SECTION("TEST that libraries can be linked") {
		std::vector<Object_Container> lib_items(3);

		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0xabcd);
			obj.machine_code.push_back(0xac00);
			obj.machine_code.push_back(0xad00);

			obj.exported_symbols.push_back(
				Exported_Symbol{ "funa", Symbol_Type::FUNCTION, 0x01 }
			);

			obj.external_references.push_back(
				External_Reference{ "funb", HI_LO_TYPE::LO_BYTE, {0x00, 0x03} }
			);
			obj.external_references.push_back(
				External_Reference{ "funb", HI_LO_TYPE::HI_BYTE, {0x04} }
			);

			lib_items.at(0).contents = obj;
		}

		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0x1011);

			obj.exported_symbols.push_back(
				Exported_Symbol{ "funb", Symbol_Type::FUNCTION, 0x03 }
			);

			obj.external_references.push_back(
				External_Reference{ "funa", HI_LO_TYPE::LO_BYTE, {0x01} }
			);
			obj.external_references.push_back(
				External_Reference{ "funa", HI_LO_TYPE::HI_BYTE, {0x02} }
			);


			lib_items.at(1).contents = obj;
		}
		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0x1000);
			obj.machine_code.push_back(0x1011);

			obj.exported_symbols.push_back(
				Exported_Symbol{ "unusedfn", Symbol_Type::FUNCTION, 0x00 }
			);

			obj.external_references.push_back(
				External_Reference{ "funa", HI_LO_TYPE::LO_BYTE, {0x01} }
			);
			obj.external_references.push_back(
				External_Reference{ "funa", HI_LO_TYPE::HI_BYTE, {0x02} }
			);

			lib_items.at(2).contents = obj;
		}

		const auto lib = make_lib(lib_items);

		// Main object will reference items from the lib
		Object_Container main_item;
		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa13);
			obj.machine_code.push_back(0x4716);
			obj.machine_code.push_back(0x7818);
			obj.machine_code.push_back(0x1214);

			// no exported symbols

			// External reference the second item in the lib, which
			// references the first to check that repeatedly checking
			// all items in the lib works.
			obj.external_references.push_back(
				External_Reference{ "funb", HI_LO_TYPE::LO_BYTE, {0x01} }
			);

			obj.exported_symbols.push_back(
				Exported_Symbol{ "main_fn", Symbol_Type::FUNCTION, 0x0002 }
			);

			main_item.contents = obj;
		}

		SECTION("Link library first") {
			std::vector<Object_Container> items_to_be_linked
			{ lib, main_item };

			// Undefined reference to "funb"
			CHECK_THROWS(link(std::move(items_to_be_linked), 0));
		}

		SECTION("Link library second") {
			std::vector<Object_Container> items_to_be_linked
			{ main_item, lib };

			const auto exe = link(std::move(items_to_be_linked), 0x0100);
			const auto& exe_contents = std::get<Executable>(exe.contents);
			const auto& code = exe_contents.machine_code;

			const Stream_Type expected_code{
				// main obj
				0xfa13,
				0x4707, // external ref lo to funb
				0x7818,
				0x1214,

				// item 2 (pulled in due to requirement for funb)
				0xfa12,
				0x1009, // external ref lo to funa
				0x1001, // external ref hi to funa
				0x1011, // funb = 0x0107

				// item 1 (pulled in due to requirement for funa)
				0xfa07, // external ref lo to funb
				0x1000, // funa = 0x0109
				0xabcd,
				0xac07, // external ref lo to funb
				0xad01, // external ref hi to funb
			};

			CHECK(code == expected_code);

			const auto& debug_external_ref = exe_contents.exported_symbols;
			CHECK(debug_external_ref.at(0).name == "main_fn");
			CHECK(debug_external_ref.at(0).type == Symbol_Type::FUNCTION);
			CHECK(debug_external_ref.at(0).offset == 0x0102);
			CHECK(debug_external_ref.at(1).name == "funb");
			CHECK(debug_external_ref.at(1).type == Symbol_Type::FUNCTION);
			CHECK(debug_external_ref.at(1).offset == 0x0107);
			CHECK(debug_external_ref.at(2).name == "funa");
			CHECK(debug_external_ref.at(2).type == Symbol_Type::FUNCTION);
			CHECK(debug_external_ref.at(2).offset == 0x0109);
			
		}

		SECTION("Link a library") {

			lib_items.push_back(main_item);

			const auto lib_container = make_lib(lib_items);
			size_t index = lib_container.contents.index();
			CHECK(index == Object_Container::Variant_Type::LIBRARY);
		}

		SECTION("Link a library with duplicate exported symbols") {
			lib_items.push_back(main_item);
			lib_items.push_back(main_item);

			CHECK_THROWS(make_lib(lib_items));
		}
	}
}

TEST_CASE("Test map creation", "[link]") {
	using namespace Object;

	Object_Container item;
	{
		Object_Type obj;
		obj.machine_code.push_back(0xfa00);
		obj.machine_code.push_back(0x4700);
		obj.machine_code.push_back(0x1241);
		obj.machine_code.push_back(0x7893);

		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::HI_BYTE, 1, 0x8700});
		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::LO_BYTE, 0, 0x0012 });

		obj.exported_symbols.emplace_back(
			Exported_Symbol{ "fun2", Symbol_Type::FUNCTION, 0x0241 });
		obj.exported_symbols.emplace_back(
			Exported_Symbol{ "fun3", Symbol_Type::FUNCTION, 0x0244 });

		item.contents = obj;
	}

	const auto exe = link({ item }, 0);

	const auto map = to_map(exe);

	const auto& obj_contents = std::get<Executable>(exe.contents);
	const auto& map_contents = std::get<Map>(map.contents);
	
	CHECK(obj_contents.exported_symbols.at(0) == map_contents.exported_symbols.at(0));
	CHECK(obj_contents.exported_symbols.at(1) == map_contents.exported_symbols.at(1));
}
