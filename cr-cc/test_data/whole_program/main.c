#include "add.h"

int main() {
	int a = 11;
	int b = 12;
	int c = 1;
	
	int sum = add(a,b);
	int not_sum = not_add(a,b);
	
	return sum + not_sum;
}
