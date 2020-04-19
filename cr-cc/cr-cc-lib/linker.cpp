#include "linker.h"
#include "cast.h"

#include <cstdint>

using namespace Object;

Object_Code link(std::vector<Object::Object_Code>&& objects)
{
	// For now we're going to assume all inputs are objects.
	// i.e. none of them are libraries or maps. This means
	// that we know that all inputs will be copied to the output.

	Object_Code output;
	//output.type = Object_Code::Object_Type::EXECUTABLE;
	//// Executables only have a single machine code section
	//auto machine_code = std::make_unique<Machine_Code>();
	//auto known_symbols = std::make_unique<Exported_Symbols>();
	//
	//
	//for (const auto& object : objects) {
	//	const auto current_offset = u16(machine_code->machine_code.size());
	//
	//	if (object.type != Object_Code::Object_Type::OBJECT) {
	//		throw std::logic_error("Link: only objects currently supported");
	//	}
	//
	//	Machine_Code* new_machine_code = nullptr;
	//	Exported_Symbols* new_exported = nullptr;
	//	External_References* new_external_ref = nullptr;
	//	Relocation* new_relocation = nullptr;
	//	for (const auto& section : object.sections) {
	//		switch (section->section_type)
	//		{
	//			case Section::Section_Type::EXPORTED_SYMBOLS:
	//				new_exported = dynamic_cast<Exported_Symbols*>(section.get());
	//				break;
	//			case Section::Section_Type::EXTERNAL_REFERENCES:
	//				new_external_ref = dynamic_cast<External_References*>(section.get());
	//				break;
	//			case Section::Section_Type::RELOCATION:
	//				new_relocation = dynamic_cast<Relocation*>(section.get());
	//				break;
	//			case Section::Section_Type::TEXT:
	//				new_machine_code = dynamic_cast<Machine_Code*>(section.get());
	//				break;
	//		default:
	//			throw std::logic_error("Link: Unexpected section");
	//		}
	//	}
	//
	//}

	return output;
}
