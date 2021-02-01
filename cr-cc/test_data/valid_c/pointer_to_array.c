// ret:2


int init_array(int * arr, int size) {
	for (int i = 0; i < size; i = i + 1) {
		*arr = 0;
		arr = arr + 1;
	}

	return 0;
}

int init_array_2(int*arr, int size) {
	for (int i = 0; i < size; i = i + 1) {
		*arr = i;
		arr = arr + 1;
	}

	return 0;
}

int inc_array(int*arr, int size) {
	for (int i = 0; i < size; i = i + 1) {
		*arr = *arr + 1;
		arr = arr + 1;
	}
}

int use_pointer_as_array(int*arr, int size) {
	for (int i = 0; i < size; i = i + 1) {
		arr[i] = arr[i] + 10;
	}
}


int static_buffer[10];
int static_buffer_size = 10;
int * static_ptr;

int init_static_buffer() {
	for (int i = 0; i < static_buffer_size; i = i + 1) {
		static_buffer[i] = 0;
	}
}

int use_static_poiter_as_array() {
	init_static_buffer();

	static_ptr = static_buffer;

	for (int i = 0; i < static_buffer_size; i = i + 1) {
		static_ptr[i] = static_ptr[i] + i;
	}

	if (static_buffer[3] != 3 ||
		static_buffer[9] != 9) {
		return 1;
	}

	return 0;
}

int use_local_pointer_as_array() {
	int local_buf[4];
	int local_size = 4;
	int * local_ptr = local_buf;

	for (int i = 0; i < local_size; i = i + 1) {
		local_buf[i] = 0;
	}

	for (int i = 0; i < local_size; i = i + 1) {
		local_ptr[i] = local_ptr[i] + i;
	}

	if (local_buf[1] != 1 ||
		local_buf[3] != 3) {

		return 1;
	}

	return 0;
}

int main() {

	int buffer[30];
	int size = 30;

	init_array(buffer, size);

	if (buffer[0] != 0 ||
		buffer[10] != 0 ||
		buffer[29] != 0) {

		return 0;
	}

	init_array_2(buffer, size);

	if (buffer[4] != 4 ||
		buffer[16] != 16 ||
		buffer[29] != 29) {

		return 0;
	}

	inc_array(buffer, size);

	if (buffer[5] != 6 ||
		buffer[18] != 19 ||
		buffer[27] != 28) {

		return 0;
	}

	use_pointer_as_array(buffer, size);

	if (buffer[2] != 13 ||
		buffer[13] != 24 ||
		buffer[29] != 40) {

		return 0;
	}

	if (use_static_poiter_as_array()) {
		return 0;
	}

	if (use_local_pointer_as_array()) {
		return 0;
	}

	// if none of the tests fail
	return 2;
}
