#include "linker.h"
#include "cast.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace Object;

static void handle_object(
	///std::vector<std::uint16_t>& output_machine_code,
	Object_Type& output_object,
	Object_Type& input_object, // Gets modified, cannot be const
	std::uint16_t link_addr
	)
{
	const std::uint16_t relocation_offset =
		u16(output_object.machine_code.size())
		+ link_addr;

	// First handle any code relocations
	{
		for (const auto& reloc : input_object.relocations) {
			std::uint16_t code = input_object.machine_code.at(reloc.location);

			// Calculate the final address and mask only the bit we need
			std::uint16_t final_address = relocation_offset + reloc.new_offset;
			if (reloc.type == HI_LO_TYPE::HI_BYTE) {
				final_address >>= 8;
			} else {
				final_address &= 0xFF;
			}

			// Apply it to the code which should have zero in the immediate
			code |= final_address;

			input_object.machine_code.at(reloc.location) = code;
		}
	}

	// Double check that this new object doesn't export any already exported symbols
	for (const auto& existing_symbol : output_object.exported_symbols) {
		for (const auto& new_symbol : input_object.exported_symbols) {
			if (existing_symbol.name == new_symbol.name &&
					new_symbol.type != Symbol_Type::DATA) {
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

static void handle_map(
	Map& output_object,
	const Map& input_map
	)
{

	// Double check that this new object doesn't export any already exported symbols
	for (const auto& existing_symbol : output_object.exported_symbols) {
		for (const auto& new_symbol : input_map.exported_symbols) {
			if (existing_symbol.name == new_symbol.name &&
				new_symbol.type != Symbol_Type::DATA) {
				throw std::logic_error("Link(): Duplicate symbol exported in map: " + existing_symbol.name);
			}
		}
	}

	// Add the exported symbols to the output
	output_object.exported_symbols.insert(
		output_object.exported_symbols.cend(),
		input_map.exported_symbols.cbegin(),
		input_map.exported_symbols.cend());
}

static void handle_lib(
	Object_Type& output_object,
	Library_Type& library,
	const std::uint16_t link_address
	)
{
	bool at_least_one_obj_used = true;
	const int size_of_lib = static_cast<int>(library.objects.size());
	// Keep track of which objects have already been used
	std::vector<bool> object_used(size_of_lib);

	while (at_least_one_obj_used) {
		at_least_one_obj_used = false;

		for (int i = 0; i < size_of_lib; ++i) {
			if (object_used.at(i)) { continue; }

			auto& obj = library.objects.at(i);

			// If this object exports a symbol that is needed, then link it
			for (const auto& symbol : obj.exported_symbols) {
				for(const auto & extern_ref : output_object.external_references) {
					if (symbol.name == extern_ref.name) {
						at_least_one_obj_used = true;
						object_used.at(i) = true;
						handle_object(output_object, obj, link_address);
						goto after;
					}
				}
			}
			after:
			;
		}
	}
}

static void apply_references(Object_Type& object, const Map& map, const std::uint16_t link_addr)
{
	std::unordered_map<std::string, Exported_Symbol> symbol_table;

	// Add all the know references to the map
	for (const auto& symbol : object.exported_symbols) {
		symbol_table[symbol.name] = symbol;
	}
	for (const auto& symbol : map.exported_symbols) {
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
			std::uint16_t word = object.machine_code.at(location - link_addr);
			word &= 0xFF00; // delete the old value
			word |= value; // apply new value
			object.machine_code.at(location - link_addr) = word;
		}
	}
}

Object_Container link(std::vector<Object::Object_Container>&& link_items, int link_addr)
{
	std::uint16_t real_link_addr = 0;
	try {
		real_link_addr = u16(link_addr);
	} catch (const std::logic_error& ) {
		throw std::logic_error("Link address out of range: " + link_addr);
	}

	// All the input objects (both objects and libs) are collected into this one,
	// then transfered to the output container
	Object_Type collector_object;
	Map collector_map;

	for (auto& item : link_items) {
		switch (item.contents.index())
		{
		case Object_Container::OBJECT:
		{
			auto& object = std::get<Object_Type>(item.contents);
			handle_object(collector_object, object, real_link_addr);
			break;
		}
		case Object_Container::LIBRARY:
		{
			auto& lib = std::get<Library_Type>(item.contents);
			handle_lib(collector_object, lib, real_link_addr);
			break;
		}
		case Object_Container::MAP:
		{
			auto& map = std::get<Map>(item.contents);
			handle_map(collector_map, map);
			break;
		}
		case Object_Container::EXECUTABLE:
			throw std::logic_error("link(): should never get here: exe is not a valid input object");
		default:
			throw std::logic_error("link(): invalid object type: " + std::to_string(item.contents.index()));
		}
	}

	// Now that we have all the objects collected, apply all the references
	apply_references(collector_object, collector_map, real_link_addr);

	Executable exe{
		real_link_addr,
		collector_object.machine_code,
		collector_object.exported_symbols
	};
	Object_Container output;
	output.contents = exe; // set the variant

	return output;
}

Object::Object_Container make_lib(const std::vector<Object::Object_Container>& objects)
{
	Library_Type library;

	std::unordered_set<std::string> exported_symbols;
	
	for (const auto& obj : objects) {
		const auto& contents = std::get<Object_Type>(obj.contents);
		library.objects.push_back(contents);

		for (const auto& symbol : contents.exported_symbols) {
			if (symbol.name != "__static_data" && exported_symbols.count(symbol.name) != 0) {
				throw std::logic_error("Duplicate symbol in library: " + symbol.name);
			}
			exported_symbols.insert(symbol.name);
		}
	}

	Object_Container output;
	output.contents = library;

	return output;
}

Object::Object_Container to_map(const Object::Object_Container& obj) {
	auto object = std::get<Executable>(obj.contents);

	Map map_contents;
	map_contents.exported_symbols = object.exported_symbols;

	Object_Container map;
	map.contents = map_contents;

	return map;
}
