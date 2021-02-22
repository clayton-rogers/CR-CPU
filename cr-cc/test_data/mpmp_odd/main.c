// Solution to MPMP How odd is Pascal's triagle
// https://www.think-maths.co.uk/pascaltriangle




#include <print_num.h>
#include <vga.h>

int SIZE = 128;
int a[128];
int b[128];


//void iterate(const int* input, int* output) {
//    output[0] = input[0]; // ^ output[-1] which is always 0
//    for (int i = 1; i < SIZE; ++i) {
//        output[i] = input[i] ^ input[i-1];
//    }
//}

// From asm
int iterate(int* input, int* output);
//int print(int* input);
int print(int* input) {
    for (int i = 0; i < SIZE; i = i + 1) {
		__print_num(input[i]);
		write_vga_char(' ');

        //std::cout << input[i];
    }
	write_vga_char(' ');

    //std::cout << std::endl;
}

int count_bits(int* input) {
    int count = 0;
    for (int i = 0; i < SIZE; i = i + 1) {
        if (input[i]) count = count + 1;
    }

    return count;
}

int meta_iterate(int count) {
    int* input = a;
    int* output = b;

    a[0] = 1;

    int bit_count = 0;


    while (count > 0) {
        count = count - 1;

        print(input);
        bit_count = bit_count + count_bits(input);

        iterate(input, output);
		int * temp = input;
		input = output;
		output = temp;
        //std::swap(input, output);
    }

    //std::cout << "Final count: " << bit_count << std::endl;
	write_vga_char('=');
	write_vga_char('=');
	write_vga_char(' ');
	__print_num(bit_count);
	write_vga_char(' ');
	write_vga_char('=');
	write_vga_char('=');

	return bit_count;
}

int main () {
	clear_screen();

    return meta_iterate(128);
}
