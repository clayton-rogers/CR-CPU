#include "label_maker.h"

std::string Label_Maker::get_next_label()
{
	return std::string(".__L") + std::to_string(label_count++);
}

std::string Label_Maker::get_fn_end_label() const
{
	return std::string() + ".__" + fn_name + "_end";
}

void Label_Maker::set_fn_name(std::string fn)
{
	this->fn_name = fn;
}

std::string Label_Maker::get_label_for_fn(std::string name) const
{
	return std::string(".") + name;
}
