#include "array.h"
#include "add.h"
#include "sub/sub.h"

int main() {
	int size = get_array_size();

	int * p = get_array();

	for (int i = 0; i < size; i = i + 1) {
		p[i] = add(p[i], i);
	}

	size = size - 1;

	for (int i = 0; i < size - 1; i = i + 2) {
		p[i] = sub(p[i+1], p[i]);
	}

	return p[10];
}
