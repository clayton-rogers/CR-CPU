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
		item1.load_address = 0;
		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x1011);
			obj.machine_code.push_back(0xabcd);

			obj.relocations.emplace_back(
				Relocation{HI_LO_TYPE::HI_BYTE, 0} );

			item1.contents = obj;
		}

		Object_Container item2;
		item2.load_address = 0;
		{
			Object_Type obj;
			obj.machine_code.push_back(0xfa12);
			obj.machine_code.push_back(0x1011);
			obj.machine_code.push_back(0x1011);
			obj.machine_code.push_back(0x1011);

			obj.relocations.emplace_back(
				Relocation{ HI_LO_TYPE::HI_BYTE, 1 });
			obj.relocations.emplace_back(
				Relocation{ HI_LO_TYPE::LO_BYTE, 2 });

			obj.exported_symbols.emplace_back(
				Exported_Symbol{ "fun", Symbol_Type::FUNCTION, 0x03 });

			item2.contents = obj;
		}

		std::vector<Object_Container> items;
		items.push_back(item1);
		items.push_back(item2);

		auto output = link(std::move(items), 0x200);

		CHECK(output.contents.index() == Object_Container::EXECUTABLE);
		static const std::uint16_t DEFAULT_LOAD_ADDR = 0x200;
		CHECK(output.load_address == DEFAULT_LOAD_ADDR);
		
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

			items.at(0).load_address = 0;
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


			items.at(1).load_address = 0;
			items.at(1).contents = obj;
		}

		auto output = link(std::move(items), 0x200);

		CHECK(output.contents.index() == Object_Container::EXECUTABLE);
		static const std::uint16_t DEFAULT_LOAD_ADDR = 0x200;
		CHECK(output.load_address == DEFAULT_LOAD_ADDR);

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

TEST_CASE("Test lib creation", "[link]") {
	using namespace Object;

	Object_Container item1;
	item1.load_address = 0;
	{
		Object_Type obj;
		obj.machine_code.push_back(0xfa12);
		obj.machine_code.push_back(0x1011);
		obj.machine_code.push_back(0xabcd);

		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::HI_BYTE, 0 });

		item1.contents = obj;
	}

	Object_Container item2;
	item2.load_address = 0;
	{
		Object_Type obj;
		obj.machine_code.push_back(0xfa12);
		obj.machine_code.push_back(0x1011);
		obj.machine_code.push_back(0x1011);
		obj.machine_code.push_back(0x1011);

		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::HI_BYTE, 1 });
		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::LO_BYTE, 2 });

		obj.exported_symbols.emplace_back(
			Exported_Symbol{ "fun", Symbol_Type::FUNCTION, 0x03 });

		item2.contents = obj;
	}

	Object_Container item3;
	item3.load_address = 0;
	{
		Object_Type obj;
		obj.machine_code.push_back(0xfa12);
		obj.machine_code.push_back(0x4787);
		obj.machine_code.push_back(0x1241);
		obj.machine_code.push_back(0x7893);

		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::HI_BYTE, 123 });
		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::LO_BYTE, 11 });

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

TEST_CASE("Test map creation", "[link]") {
	using namespace Object;

	Object_Container item;
	item.load_address = 0x2313;
	{
		Object_Type obj;
		obj.machine_code.push_back(0xfa12);
		obj.machine_code.push_back(0x4787);
		obj.machine_code.push_back(0x1241);
		obj.machine_code.push_back(0x7893);

		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::HI_BYTE, 1 });
		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::LO_BYTE, 0 });

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
