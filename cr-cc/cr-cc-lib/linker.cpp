#include "linker.h"
#include "cast.h"

#include <cstdint>
#include <vector>
#include <unordered_map>

using namespace Object;

static const std::uint16_t DEFAULT_LINKER_LOAD_ADDRESS = 0x0200;

static void handle_object(
	///std::vector<std::uint16_t>& output_machine_code,
	Object_Type& output_object,
	Object_Type& input_object,
	std::uint16_t objects_load_address
	)
{
	const std::uint16_t relocation_offset = u16(output_object.machine_code.size())
		+ DEFAULT_LINKER_LOAD_ADDRESS
		- objects_load_address;

	// First handle any code relocations
	{
		for (const auto& reloc : input_object.relocations) {
			std::uint16_t offset = input_object.machine_code.at(reloc.offset);

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
			std::uint16_t word = input_object.machine_code.at(reloc.offset);
			word &= 0xFF00; // delete the old value
			word |= offset; // apply new value
			input_object.machine_code.at(reloc.offset) = word;
		}
	}

	// Double check that this new object doesn't export any already exported symbols
	for (const auto& existing_symbol : output_object.exported_symbols) {
		for (const auto& new_symbol : input_object.exported_symbols) {
			if (existing_symbol.name == new_symbol.name) {
				throw std::logic_error("Link(): Duplicate symbol exported: " + existing_symbol.name);
			}
		}
	}

	// Relocate exported symbols
	for (auto& symbol : input_object.exported_symbols) {
		symbol.offset += relocation_offset;
	}

	// Add the exported symbols to the output
	output_object.exported_symbols.insert(
		output_object.exported_symbols.cend(),
		input_object.exported_symbols.cbegin(),
		input_object.exported_symbols.cend());

	// Relocate external references
	for (auto& ref : input_object.external_references) {
		for (auto& location : ref.locations) {
			location += relocation_offset;
		}
	}

	// Add the external references to the output
	output_object.external_references.insert(
		output_object.external_references.cend(),
		input_object.external_references.cbegin(),
		input_object.external_references.cend());

	// finally just append the new object code to the existing
	output_object.machine_code.insert(
		output_object.machine_code.cend(),
		input_object.machine_code.cbegin(),
		input_object.machine_code.cend());
}

static void apply_references(Object_Type& object)
{
	std::unordered_map<std::string, Exported_Symbol> symbol_table;

	// Add all the know references to the map
	for (const auto& symbol : object.exported_symbols) {
		symbol_table[symbol.name] = symbol;
	}

	// Try to resolve all the references
	for (const auto& ref : object.external_references) {
		if (symbol_table.count(ref.name) == 0) {
			throw std::logic_error("Undefined external reference: " + ref.name);
		}

		const auto& symbol = symbol_table.at(ref.name);
		std::uint16_t value = (ref.type == HI_LO_TYPE::HI_BYTE) ? symbol.offset >> 8 : symbol.offset & 0xFF;

		for (const auto& location : ref.locations) {
			// TODO kind of a hack, should be a proper way to get the address
			std::uint16_t word = object.machine_code.at(location - DEFAULT_LINKER_LOAD_ADDRESS);
			word &= 0xFF00; // delete the old value
			word |= value; // apply new value
			object.machine_code.at(location - DEFAULT_LINKER_LOAD_ADDRESS) = word;
		}
	}
}

Object_Container link(std::vector<Object::Object_Container>&& link_items)
{
	// For now we're going to assume all inputs are objects.
	// i.e. none of them are libraries or maps. This means
	// that we know that all inputs will be copied to the output.

	// All the input objects are collected into this one, then
	// transfered to the output container
	Object_Type collector_object;

	for (auto& item : link_items) {
		switch (item.contents.index())
		{
		case Object_Container::OBJECT:
		{
			auto& object = std::get<Object_Type>(item.contents);
			handle_object(collector_object, object, item.load_address);
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

	// Now that we have all the objects collected, apply all the references
	apply_references(collector_object);

	Executable exe{
		collector_object.machine_code,
		collector_object.exported_symbols
	};
	Object_Container output;
	output.contents = exe; // set the variant
	output.load_address = DEFAULT_LINKER_LOAD_ADDRESS;

	return output;
}
