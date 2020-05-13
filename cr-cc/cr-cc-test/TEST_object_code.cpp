#include "object_code.h"

#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.h"

TEST_CASE("Test object serialization", "[obj]") {
	using namespace Object;

	Object_Container container;
	container.load_address = 0xfa56;
	{
		Object_Type obj;

		obj.exported_symbols.emplace_back(
			Exported_Symbol{ "fun", Symbol_Type::FUNCTION, 0x03 });
		obj.exported_symbols.emplace_back(
			Exported_Symbol{ "global_variable", Symbol_Type::VARIABLE, 0x3213 });

		obj.external_references.push_back(
			External_Reference{ "funb", HI_LO_TYPE::LO_BYTE, {0x01, 0x03, 0xFFFF} });
		obj.external_references.push_back(
			External_Reference{ "funb", HI_LO_TYPE::HI_BYTE, {0x04} });
		obj.external_references.push_back(
			External_Reference{ "add_fn", HI_LO_TYPE::HI_BYTE, {0xab41, 0xc8e2} });

		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::HI_BYTE, 0x1234 });
		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::LO_BYTE, 0x7FFF });

		obj.machine_code.push_back(0xfa12);
		obj.machine_code.push_back(0x1234);
		obj.machine_code.push_back(0xfabc);
		obj.machine_code.push_back(0xbefa);

		container.contents = obj;
	}

	auto stream = container.to_stream();

	auto new_container = Object_Container::from_stream(stream);

	CHECK(new_container == container);
}

TEST_CASE("Test library serialization", "[obj]") {
	using namespace Object;

	Object_Container container;
	container.load_address = 0xfa56;

	Library_Type lib;
	{
		Object_Type obj;

		obj.exported_symbols.emplace_back(
			Exported_Symbol{ "fun", Symbol_Type::FUNCTION, 0x03 });
		obj.exported_symbols.emplace_back(
			Exported_Symbol{ "global_variable", Symbol_Type::VARIABLE, 0x3213 });

		obj.external_references.push_back(
			External_Reference{ "funb", HI_LO_TYPE::LO_BYTE, {0x01, 0x03, 0xFFFF} });
		obj.external_references.push_back(
			External_Reference{ "funb", HI_LO_TYPE::HI_BYTE, {0x04} });
		obj.external_references.push_back(
			External_Reference{ "add_fn", HI_LO_TYPE::HI_BYTE, {0xab41, 0xc8e2} });

		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::HI_BYTE, 0x1234 });
		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::LO_BYTE, 0x7FFF });

		obj.machine_code.push_back(0xfa12);
		obj.machine_code.push_back(0x1234);
		obj.machine_code.push_back(0xfabc);
		obj.machine_code.push_back(0xbefa);

		lib.objects.push_back(obj);
	}

	{
		Object_Type obj;

		obj.exported_symbols.emplace_back(
			Exported_Symbol{ "fufdsfsan", Symbol_Type::FUNCTION, 0xe342 });
		obj.exported_symbols.emplace_back(
			Exported_Symbol{ "global_variable", Symbol_Type::VARIABLE, 0x3213 });

		obj.external_references.push_back(
			External_Reference{ "funb", HI_LO_TYPE::LO_BYTE, {0x01, 0x03, 0xFFFF} });
		obj.external_references.push_back(
			External_Reference{ "funb", HI_LO_TYPE::HI_BYTE, {0x04} });
		obj.external_references.push_back(
			External_Reference{ "add_fn", HI_LO_TYPE::HI_BYTE, {0xab41, 0xc8e2} });

		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::HI_BYTE, 0x1234 });
		obj.relocations.emplace_back(
			Relocation{ HI_LO_TYPE::LO_BYTE, 0x7FFF });

		obj.machine_code.push_back(0xfa12);
		obj.machine_code.push_back(0x1234);
		obj.machine_code.push_back(0xfabc);
		obj.machine_code.push_back(0xbefa);
		obj.machine_code.push_back(0xfa12);
		obj.machine_code.push_back(0x1234);
		obj.machine_code.push_back(0xfabc);
		obj.machine_code.push_back(0xbefa);
		obj.machine_code.push_back(0xfa12);
		obj.machine_code.push_back(0x1234);
		obj.machine_code.push_back(0xfabc);
		obj.machine_code.push_back(0xbefa);

		lib.objects.push_back(obj);
	}

	container.contents = lib;

	auto stream = container.to_stream();

	auto new_container = Object_Container::from_stream(stream);

	CHECK(new_container == container);
}

TEST_CASE("Test executable serialization", "[obj]") {
	using namespace Object;

	Object_Container container;
	container.load_address = 0xfa56;
	{
		Executable exe;

		exe.exported_symbols.emplace_back(
			Exported_Symbol{ "fun", Symbol_Type::FUNCTION, 0x03 });
		exe.exported_symbols.emplace_back(
			Exported_Symbol{ "global_variable", Symbol_Type::VARIABLE, 0x3213 });

		exe.machine_code.push_back(0xfa12);
		exe.machine_code.push_back(0x1234);
		exe.machine_code.push_back(0xfabc);
		exe.machine_code.push_back(0xbefa);
		exe.machine_code.push_back(0xfa12);
		exe.machine_code.push_back(0x1234);
		exe.machine_code.push_back(0xfabc);
		exe.machine_code.push_back(0xbefa);

		container.contents = exe;
	}

	auto stream = container.to_stream();

	auto new_container = Object_Container::from_stream(stream);

	CHECK(new_container == container);
}

TEST_CASE("Test map serialization", "[obj]") {
	using namespace Object;

	Object_Container container;
	container.load_address = 0xfa56;
	{
		Map exe;

		exe.exported_symbols.emplace_back(
			Exported_Symbol{ "fun", Symbol_Type::FUNCTION, 0x03 });
		exe.exported_symbols.emplace_back(
			Exported_Symbol{ "global_variable", Symbol_Type::VARIABLE, 0x3213 });

		container.contents = exe;
	}

	auto stream = container.to_stream();

	auto new_container = Object_Container::from_stream(stream);

	CHECK(new_container == container);
}
