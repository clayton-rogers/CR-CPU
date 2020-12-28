// ret: 2

int simple_pointer() {
	int * a;

	int b = 12;

	a = &b;

	return *a == 12;
}

int pointer_init() {
	int b = 2;
	int * a = &b;

	return *a == 2;
}

int static_var = 11;

int pointer_to_static() {
	int * a = &static_var;

	return *a == 11;
}

int assign_through_pointer() {
	int var = 11;
	int ptr = &var;

	*ptr = 14;

	return var == 14;
}

int assign_to_static_var_through_pointer() {
	static_var = 13;
	int ptr = &static_var;

	*ptr = 14;

	return static_var == 14;
}

int comparison_of_pointers() {
	int a = 3213;
	int ptr_a = &a;
	int ptr_b = &a;

	return ptr_a == ptr_b;
}

int main() {
	if (!simple_pointer()) {
		return 1;
	}
	if (!pointer_init()) {
		return 1;
	}
	if (!pointer_to_static()) {
		return 1;
	}
	if (!assign_through_pointer()) {
		return 1;
	}
	if(!assign_to_static_var_through_pointer()) {
		return 1;
	}
	if (!comparison_of_pointers()) {
		return 1;
	}

	return 2;
}
