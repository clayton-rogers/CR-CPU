#include "object_code.h"

#include "cast.h"

#include <iterator>

using namespace Object;

Stream_Type Section::to_stream() const
{
	auto child_stream = child_to_stream();

	Stream_Type section_stream;
	section_stream.push_back(u16(section_type));
	section_stream.push_back(u16(child_stream.size()));
	section_stream.insert(section_stream.cend(),
		std::make_move_iterator(child_stream.cbegin()),
		std::make_move_iterator(child_stream.cend()));
	
	return section_stream;
}

Stream_Type External_References::child_to_stream() const
{
	Stream_Type stream;
	for (const auto& ref : references) {
		for (const auto& character : ref.name) {
			stream.push_back(static_cast<std::uint16_t>(character));
		}
		stream.push_back(0); // null to terminate string
		stream.push_back(static_cast<std::uint16_t>(ref.type));
		stream.insert(stream.cend(), ref.locations.cbegin(), ref.locations.cend());
	}

	return stream;
}

Stream_Type Relocation::child_to_stream() const
{
	// Just return a copy of the locations
	return relocation_locations;
}

Stream_Type Exported_Symbols::child_to_stream() const
{
	Stream_Type stream;
	for (const auto& symbol : symbols) {
		for (const auto& character : symbol.name) {
			stream.push_back(static_cast<std::uint16_t>(character));
		}
		stream.push_back(0); // null to terminate string
		stream.push_back(static_cast<std::uint16_t>(symbol.type));
		stream.push_back(symbol.offset);
	}

	return stream;
}

Stream_Type Machine_Code::child_to_stream() const
{
	// Just return a copy of the machine code
	return machine_code;
}

Stream_Type Object_Code::to_object_code() const
{
	std::vector<std::uint16_t> stream;

	auto stream_cat = [](Stream_Type& a, const Stream_Type& b) {
		return a.insert(a.cend(), b.cbegin(), b.cend());
	};

	auto object_to_stream = [&stream_cat](Stream_Type& s, const Object_Type& obj) {
		auto temp = obj.exports.to_stream();
		stream_cat(s, temp);
		temp = obj.references.to_stream();
		stream_cat(s, temp);
		temp = obj.relocations.to_stream();
		stream_cat(s, temp);
		temp = obj.machine_code.to_stream();
		stream_cat(s, temp);
	};

	// object code header
	stream.push_back(MAGIC);
	stream.push_back(object_version);
	stream.push_back(u16(contents.index())); // type of object
	stream.push_back(load_address);
	stream.push_back(0); // reserved
	stream.push_back(0); // reserved
	stream.push_back(0); // reserved
	stream.push_back(0); // placeholder for size of sections

	// contents
	switch (contents.index()) {
	case OBJECT:
	{
		const auto& object = std::get<Object_Type>(contents);
		object_to_stream(stream, object);
		break;
	}
	case LIBRARY:
	{
		const auto& lib = std::get<Library_Type>(contents);
		for (const auto& object : lib.objects) {
			object_to_stream(stream, object);
		}
		break;
	}
	case EXECUTABLE:
	{
		const auto& exe = std::get<Executable>(contents);
		auto temp = exe.machine_code.to_stream();
		stream_cat(stream, temp);
		break;
	}
	case MAP:
	{
		const auto& map = std::get<Map>(contents);
		auto temp = map.exports.to_stream();
		stream_cat(stream, temp);
		break;
	}
	}

	static constexpr auto OC_HEADER_SIZE = 4;
	stream.at(3) = u16(stream.size() - OC_HEADER_SIZE);

	return stream;
}
