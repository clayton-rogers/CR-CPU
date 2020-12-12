#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>


enum class Specifiers {
	// Storage class
	TYPEDEF,
	EXTERN,
	STATIC,
	AUTO,
	// REGISTER deliberately omitted

	// Type specifier
	VOID,
	INT,
	// TODO other integral types
	STRUCT,
	ENUM,
	TYPEDEF_NAME, // ????

	// Type qualifier
	CONST,
	VOLATILE,
};

struct Declaration_Specifier {
	std::vector<Specifiers> specifier_list;

	bool contains(Specifiers specifier);
};

enum class Broad_Type {
	INTEGRAL,
	POINTER,
	ARRAY,
	STRUCT,
	// FUNCTION, ???
};

struct Abstract_Declarator {
	// Number of pointers are not part of the direct declarator
	// TODO will need to handle which are const
	int num_pointers = 0;

	// Nested declarator
	// All except ident use this
	// Ex. an array contains a nested decl which eventually has an ident
	std::shared_ptr<Abstract_Declarator> nested_declarator;

	// Array
	std::vector<int> array_sizes;

	// Function
	// ???
};

class Type {
public:
	Type(const Declaration_Specifier& specifiers, const Abstract_Declarator& declarator);

	int get_size() const { return size; }
	Broad_Type get_broad_type() const { return broad_type; }

	bool is_same(std::shared_ptr<Type> other);

private:
	Broad_Type broad_type;
	int size;

	Declaration_Specifier declaration_specifiers;
	Abstract_Declarator abstract_declarator;
};
