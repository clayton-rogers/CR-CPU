#pragma once

#include <string>

class Label_Maker {
public:
	std::string get_next_label();
	std::string get_fn_end_label() const;
	void		set_fn_name(std::string end);
	std::string get_label_for_fn(std::string fn_name) const;
private:
	int label_count = 0;
	std::string fn_name;
};
