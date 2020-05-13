#include "object_code.h"

#include "cast.h"

#include <iterator>

using namespace Object;

class Stream_Type_Iterator {
public:
	Stream_Type_Iterator(const Stream_Type& s) : stream(s) {}

	std::uint16_t get_next() {
		auto value = stream.at(offset);
		++offset;
		return value;
	}
	std::uint16_t peek_next() {
		auto value = stream.at(offset);
		return value;
	}
	void set_offset(int value) {
		offset = value;
	}

private:
	int offset = 0;
	const Stream_Type& stream;
};

template <typename T>
T from_stream(Stream_Type_Iterator& s) {
	//static_assert (false, "Forgot to implement specialization of from_stream()");
}

static void stream_cat(Stream_Type& a, const Stream_Type& b) {
	a.insert(a.cend(), b.cbegin(), b.cend());
}

static Stream_Type to_stream(const std::string& str) {
	Stream_Type s;
	for (const auto& character : str) {
		s.push_back(u16(character));
	}
	s.push_back(0); // null terminate
	return s;
}

template<>
std::string from_stream(Stream_Type_Iterator& s) {
	std::string my_string;
	while (true) {
		auto value = s.get_next();
		if (value == 0) {
			break;
		}
		if ( (value & 0x00FF) != value) {
			throw std::logic_error("Invalid character in string");
		}

		my_string.append(1, static_cast<char>(value));
	}
	return my_string;
}

static Stream_Type to_stream(const Symbol_Type& type) {
	return Stream_Type{ u16(type) };
}

template<>
Symbol_Type from_stream(Stream_Type_Iterator& s) {
	auto value = s.get_next();
	switch (value) {
	case 0:
		return Symbol_Type::FUNCTION;
	case 1:
		return Symbol_Type::VARIABLE;
	default:
		throw std::logic_error("Invalid conversion to Symbol type from " + std::to_string(value));
	}
}

static Stream_Type to_stream(const Exported_Symbol& symbol) {
	Stream_Type s;
	stream_cat(s, to_stream(symbol.name));
	stream_cat(s, to_stream(symbol.type));
	s.push_back(symbol.offset);

	return s;
}

template<>
Exported_Symbol from_stream(Stream_Type_Iterator& s) {
	Exported_Symbol symbol;
	symbol.name = from_stream<std::string>(s);
	symbol.type = from_stream<Symbol_Type>(s);
	symbol.offset = s.get_next();
	return symbol;
}

static Stream_Type to_stream(const HI_LO_TYPE& type) {
	return Stream_Type{ u16(type) };
}

template<>
HI_LO_TYPE from_stream(Stream_Type_Iterator& s) {
	auto value = s.get_next();
	switch (value)
	{
	case 0:
		return HI_LO_TYPE::LO_BYTE;
	case 1:
		return HI_LO_TYPE::HI_BYTE;
	default:
		throw std::logic_error("Invalid conversion to HI_LO_TYPE from " + std::to_string(value));
	}
}

static Stream_Type to_stream(const External_Reference& reference) {
	Stream_Type s;
	stream_cat(s, to_stream(reference.name));
	stream_cat(s, to_stream(reference.type));
	s.push_back(u16(reference.locations.size()));
	for (const auto& location : reference.locations) {
		s.push_back(location);
	}

	return s;
}

template<>
External_Reference from_stream(Stream_Type_Iterator& s) {
	External_Reference reference;
	reference.name = from_stream<std::string>(s);
	reference.type = from_stream<HI_LO_TYPE>(s);
	const int number_of_references = s.get_next();
	for (int i = 0; i < number_of_references; ++i) {
		reference.locations.push_back(
			s.get_next()
		);
	}

	return reference;
}

static Stream_Type to_stream(const Relocation& relocation) {
	std::uint16_t value = relocation.offset;

	// We cheat with relocations an assume that they will always be smaller than 32k
	// so that we can use the upper bit for the type of relocation
	if (value > 0x7FFF) {
		throw std::logic_error("Internal error: relocation offset too great");
	}
	if (relocation.type == HI_LO_TYPE::HI_BYTE) {
		value |= 0x8000;
	}

	return Stream_Type{ value };
}

template<>
Relocation from_stream(Stream_Type_Iterator& s) {
	auto value = s.get_next();

	Relocation relocation;
	if (0x8000 & value) {
		relocation.type = HI_LO_TYPE::HI_BYTE;
		value &= 0x7FFF;
	} else {
		relocation.type = HI_LO_TYPE::LO_BYTE;
	}

	relocation.offset = value;

	return relocation;
}

static Stream_Type to_stream(const Object_Type& obj) {
	Stream_Type s;
	s.push_back(u16(obj.exported_symbols.size()));
	for (const auto& symbol : obj.exported_symbols) {
		stream_cat(s, to_stream(symbol));
	}
	s.push_back(u16(obj.external_references.size()));
	for (const auto& reference : obj.external_references) {
		stream_cat(s, to_stream(reference));
	}
	s.push_back(u16(obj.relocations.size()));
	for (const auto& relocation : obj.relocations) {
		stream_cat(s, to_stream(relocation));
	}
	s.push_back(u16(obj.machine_code.size()));
	for (const auto& instruction : obj.machine_code) {
		s.push_back(instruction);
	}

	return s;
}

template<>
Object_Type from_stream(Stream_Type_Iterator& s) {
	Object_Type o;

	const int export_symbol_size = s.get_next();
	for (int i = 0; i < export_symbol_size; ++i) {
		o.exported_symbols.push_back(
			from_stream<Exported_Symbol>(s)
		);
	}

	const int external_refernce_size = s.get_next();
	for (int i = 0; i < external_refernce_size; ++i) {
		o.external_references.push_back(
			from_stream<External_Reference>(s)
		);
	}

	const int relocations_size = s.get_next();
	for (int i = 0; i < relocations_size; ++i) {
		o.relocations.push_back(
			from_stream<Relocation>(s)
		);
	}

	const int machine_code_size = s.get_next();
	for (int i = 0; i < machine_code_size; ++i) {
		o.machine_code.push_back(
			s.get_next()
		);
	}

	return o;
}

static Stream_Type to_stream(const Library_Type& lib) {
	Stream_Type s;
	s.push_back(u16(lib.objects.size()));
	for (const auto& object : lib.objects) {
		stream_cat(s, ::to_stream(object));
	}
	return s;
}

template<>
Library_Type from_stream(Stream_Type_Iterator& s) {
	Library_Type lib;
	int number_of_objects = s.get_next();
	for (int i = 0; i < number_of_objects; ++i) {
		lib.objects.push_back(
			from_stream<Object_Type>(s)
		);
	}

	return lib;
}

static Stream_Type to_stream(const Executable& exe) {
	Stream_Type s;
	s.push_back(u16(exe.machine_code.size()));
	for (const auto& instruction : exe.machine_code) {
		s.push_back(instruction);
	}
	s.push_back(u16(exe.exported_symbols.size()));
	for (const auto& symbol : exe.exported_symbols) {
		stream_cat(s, to_stream(symbol));
	}

	return s;
}

template<>
Executable from_stream(Stream_Type_Iterator& s) {
	Executable exe;
	const int machine_code_size = s.get_next();
	for (int i = 0; i < machine_code_size; ++i) {
		exe.machine_code.push_back(
			s.get_next()
		);
	}

	const int exported_symbol_size = s.get_next();
	for (int i = 0; i < exported_symbol_size; ++i) {
		exe.exported_symbols.push_back(
			from_stream<Exported_Symbol>(s)
		);
	}

	return exe;
}

static Stream_Type to_stream(const Map& map) {
	Stream_Type s;
	s.push_back(u16(map.exported_symbols.size()));
	for (const auto& symbol : map.exported_symbols) {
		stream_cat(s, to_stream(symbol));
	}

	return s;
}

template<>
Map from_stream(Stream_Type_Iterator& s) {
	Map m;
	const int exported_symbol_size = s.get_next();
	for (int i = 0; i < exported_symbol_size; ++i) {
		m.exported_symbols.push_back(
			from_stream<Exported_Symbol>(s)
		);
	}

	return m;
}

Stream_Type Object_Container::to_stream() const
{
	std::vector<std::uint16_t> stream;

	// object code header
	stream.push_back(MAGIC);
	stream.push_back(OBJECT_VERSION);
	stream.push_back(u16(contents.index())); // type of object
	stream.push_back(load_address);
	stream.push_back(0); // reserved
	stream.push_back(0); // reserved
	stream.push_back(0); // reserved
	stream.push_back(0); // placeholder for size of sections

	const auto OC_HEADER_SIZE = stream.size();

	// contents
	switch (contents.index()) {
	case OBJECT:
	{
		const auto& object = std::get<Object_Type>(contents);
		stream_cat(stream, ::to_stream(object));
		break;
	}
	case LIBRARY:
	{
		const auto& lib = std::get<Library_Type>(contents);
		stream_cat(stream, ::to_stream(lib));
		break;
	}
	case EXECUTABLE:
	{
		const auto& exe = std::get<Executable>(contents);
		stream_cat(stream, ::to_stream(exe));
		break;
	}
	case MAP:
	{
		const auto& map = std::get<Map>(contents);
		stream_cat(stream, ::to_stream(map));
		break;
	}
	}
	
	stream.at(7) = u16(stream.size() - OC_HEADER_SIZE);

	return stream;
}

static void check_header(const Stream_Type& s) {
	if (s.at(0) != Object_Container::MAGIC) {
		throw std::logic_error("Magic value does not match!");
	}
	if (s.at(1) != Object_Container::OBJECT_VERSION) {
		throw std::logic_error(std::string("Version does not match, expected: ") +
			std::to_string(Object_Container::OBJECT_VERSION) +
			" got: " + std::to_string(s.at(1)));
	}
}

Object_Container Object_Container::from_stream(const Stream_Type& s) {
	check_header(s);

	Object_Container c;
	c.load_address = s.at(3);
	const auto size = s.at(7);
	int offset = 8; // size of header

	if ((size + offset) != static_cast<int>(s.size())) {
		throw std::logic_error("Object size mismatch");
	}

	auto type = static_cast<Variant_Type>(s.at(2));

	Stream_Type_Iterator stream_iterator(s);
	stream_iterator.set_offset(offset);

	switch (type) {
	case OBJECT:
	{
		c.contents = ::from_stream<Object_Type>(stream_iterator);
		break;
	}
	case LIBRARY:
	{
		c.contents = ::from_stream<Library_Type>(stream_iterator);
		break;
	}
	case EXECUTABLE:
	{
		c.contents = ::from_stream<Executable>(stream_iterator);
		break;
	}
	case MAP:
	{
		c.contents = ::from_stream<Map>(stream_iterator);
		break;
	}
	}

	return c;
}

const std::uint16_t Object_Container::MAGIC; // ascii "CR"
const std::uint16_t Object_Container::OBJECT_VERSION;


// **********************************************************************
// Operators
// **********************************************************************
bool Object::operator==(const Object_Container& a, const Object_Container& b) {
	return
		a.load_address == b.load_address &&
		a.contents == b.contents;
}

bool Object::operator==(const Object_Type& a, const Object_Type& b) {
	return
		a.exported_symbols == b.exported_symbols &&
		a.external_references == b.external_references &&
		a.relocations == b.relocations &&
		a.machine_code == b.machine_code;
}

bool Object::operator==(const Library_Type& a, const Library_Type& b) {
	return a.objects == b.objects;
}

bool Object::operator==(const Executable& a, const Executable& b) {
	return
		a.machine_code == b.machine_code &&
		a.exported_symbols == b.exported_symbols;
}

bool Object::operator==(const Map& a, const Map& b) {
	return a.exported_symbols == b.exported_symbols;
}

bool Object::operator==(const Object::Relocation& a, const Object::Relocation& b) {
	if (a.type != b.type) {
		return false;
	} else if (a.offset != b.offset) {
		return false;
	} else {
		return true;
	}
}

bool Object::operator==(const Object::External_Reference& a, const Object::External_Reference& b) {
	if (a.name != b.name) {
		return false;
	} else if (a.type != b.type) {
		return false;
	} else if (a.locations != b.locations) {
		return false;
	} else {
		return true;
	}
}

bool Object::operator==(const Object::Exported_Symbol& a, const Object::Exported_Symbol& b) {
	if (a.name != b.name) {
		return false;
	} else if (a.type != b.type) {
		return false;
	} else if (a.offset != b.offset) {
		return false;
	} else {
		return true;
	}
}
