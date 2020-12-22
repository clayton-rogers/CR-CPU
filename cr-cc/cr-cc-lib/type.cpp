#include "type.h"

#include <stdexcept>

// Sizes of built ins in words (16 bits)
static const int PTR_SIZE = 1;
static const int INT_SIZE = 1;

static int calc_struct_size(const Declaration_Specifier& specifer) {

	throw std::logic_error("struct size not yet implemented");
}

Type::Type(const Declaration_Specifier& specifiers, const Abstract_Declarator& declarator)
	: declaration_specifiers(specifiers), abstract_declarator(declarator) {

	// Need to determine size and broad type

	bool specifier_type_determined = false;
	Specifiers specifier_type = Specifiers::INT;
	for (const auto& specifier : specifiers.specifier_list) {
		switch (specifier) {
		case Specifiers::INT:
		case Specifiers::VOID:
		case Specifiers::STRUCT:
		case Specifiers::ENUM:
		case Specifiers::TYPEDEF_NAME:
			if (specifier_type_determined) {
				throw std::logic_error("Tried to redefied type");
			}
			specifier_type = specifier;
			specifier_type_determined = true;
		default: ; // ignore
		}
	}

	if (!specifier_type_determined) {
		throw std::logic_error("Could not determine specifier type");
	}

	if (declarator.num_pointers != 0) {
		broad_type = Broad_Type::POINTER;
		size = PTR_SIZE;
	} else if (declarator.array_sizes.size() != 0) {
		broad_type = Broad_Type::ARRAY;
		size = PTR_SIZE;
	} else if (specifier_type == Specifiers::STRUCT) {
		broad_type = Broad_Type::STRUCT;
		size = calc_struct_size(specifiers);
	} else {
		broad_type = Broad_Type::INTEGRAL;
		if (specifier_type == Specifiers::INT) {
			size = INT_SIZE;
		} else if (specifier_type == Specifiers::VOID) {
			size = 0;
		} else {
			throw std::logic_error("Got integral type other than int or void");
		}
	}
}


bool operator==(const Declaration_Specifier& a, const Declaration_Specifier& b) {
	if (a.specifier_list.size() != b.specifier_list.size()) {
		return false;
	}

	// Make sure the set of specifiers is the same, but order doesn't matter

	std::vector<int> found_in_second(a.specifier_list.size(), 0);

	int offset = 0;
	for (const auto& specifier_a : a.specifier_list) {
		for (const auto& specifier_b : b.specifier_list) {
			if (specifier_a == specifier_b) {
				found_in_second.at(offset)++;
			}
		}
		offset++;
	}

	for (const auto& found : found_in_second) {
		if (found != 1) {
			return false;
		}
	}

	return true;
}

bool operator==(const Abstract_Declarator& a, const Abstract_Declarator& b) {
	if (a.num_pointers != b.num_pointers) {
		return false;
	}

	if (a.array_sizes != b.array_sizes) {
		return false;
	}

	if (a.nested_declarator) {
		if (b.nested_declarator) {
			return *a.nested_declarator == *b.nested_declarator;
		} else {
			return false;
		}
	} else {
		if (b.nested_declarator) {
			return false;
		} else {
			return true; // if both are nullptr
		}
	}

	throw std::logic_error("operator==(Abstract_Declarator) should never get here");
}

bool Type::is_same(std::shared_ptr<Type> other)
{
	if (this->size != other->size) {
		return false;
	}
	if (this->broad_type != other->broad_type) {
		return false;
	}

	if (!(this->declaration_specifiers == other->declaration_specifiers)) {
		return false;
	}

	if (!(this->abstract_declarator == other->abstract_declarator)) {
		return false;
	}

	return true;
}

bool Declaration_Specifier::contains(Specifiers specifier) {
	for (Specifiers s : specifier_list) {
		if (s == specifier) {
			return true;
		}
	}

	return false;
}

const std::shared_ptr<Type> VOID_TYPE = std::make_shared<Type>(Declaration_Specifier{ { Specifiers::VOID } }, Abstract_Declarator());
const std::shared_ptr<Type> INT_TYPE = std::make_shared<Type>(Declaration_Specifier{ { Specifiers::INT } }, Abstract_Declarator());
