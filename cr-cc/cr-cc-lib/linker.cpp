#include "linker.h"
#include "cast.h"

#include <cstdint>
#include <vector>

using namespace Object;

static const std::uint16_t DEFAULT_LINKER_LOAD_ADDRESS = 0x0200;

static void handle_object(
	std::vector<std::uint16_t>& output_machine_code,
	Object_Type& object,
	std::uint16_t objects_load_address
	)
{
	const std::uint16_t relocation_offset = u16(output_machine_code.size())
		+ DEFAULT_LINKER_LOAD_ADDRESS
		- objects_load_address;

	// First handle any code relocations
	{
		for (const auto& reloc : object.relocations) {
			std::uint16_t offset = object.machine_code.at(reloc.offset);

			// mask out the byte we care about
			offset &= 0xFF;
			if (reloc.type == HI_LO_TYPE::HI_BYTE) {
				offset <<= 8;
			}

			// apply the offset and mask back the byte we want
			offset += relocation_offset;
			if (reloc.type == HI_LO_TYPE::HI_BYTE) {
				offset >>= 8;
			} else {
				offset &= 0xFF;
			}

			// write the byte back to the machine code
			std::uint16_t word = object.machine_code.at(reloc.offset);
			word &= 0xFF00; // delete the old value
			word |= offset; // apply new value
			object.machine_code.at(reloc.offset) = word;
		}
	}

	// Next exported symbol relocations
	{
		for (auto& symbol : object.exported_symbols) {
			symbol.offset += relocation_offset;
		}
	}

	// finally just append the new object code to the existing
	{
		const auto& new_mach = object.machine_code;
		output_machine_code.insert(output_machine_code.cend(), new_mach.cbegin(), new_mach.cend());
	}
}

Object_Container link(std::vector<Object::Object_Container>&& link_items)
{
	// For now we're going to assume all inputs are objects.
	// i.e. none of them are libraries or maps. This means
	// that we know that all inputs will be copied to the output.

	Object_Container output;
	output.contents = Executable(); // set the variant
	output.load_address = DEFAULT_LINKER_LOAD_ADDRESS;

	auto& machine = std::get<Executable>(output.contents).machine_code;

	for (auto& item : link_items) {
		switch (item.contents.index())
		{
		case Object_Container::OBJECT:
		{
			auto& object = std::get<Object_Type>(item.contents);
			handle_object(machine, object, item.load_address);
			break;
		}
		case Object_Container::LIBRARY:
			throw std::logic_error("link(): libraries not implemented yet");
			break;
		case Object_Container::MAP:
			throw std::logic_error("link(): map not implemented yet");
			break;
		case Object_Container::EXECUTABLE:
			throw std::logic_error("link(): should never get here: exe is not a valid input object");
		default:
			throw std::logic_error("link(): invalid object type: " + std::to_string(item.contents.index()));
		}
	}




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
