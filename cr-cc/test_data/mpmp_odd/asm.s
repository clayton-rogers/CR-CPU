

#void iterate(const int* input, int* output) {
#    output[0] = input[0]; // ^ output[-1] which is always 0
#    for (int i = 1; i < SIZE; ++i) {
#        output[i] = input[i] ^ input[i-1];
#    }
#}

# INPUT:
#   ra = (int*) input
#   rb = (int*) output

.iterate:
.export iterate
push rb
push rb
push ra

# sp + 1 = output
# sp + 0 = input
.static 1 iterate_stored_sp
loada .iterate_stored_sp
store sp .iterate_stored_sp

mov sp ra
mov rp rb

# sp = input
# rp = output

#    output[0] = input[0]; // ^ output[-1] which is always 0
load.sp ra 0
store.rp ra 0


#    for (int i = 1; i < SIZE; ++i) {
#        output[i] = input[i] ^ input[i-1];
#    }
loadi ra 0xFF
.static 1 stored_i
loada .stored_i
store ra .stored_i
add sp 1
add rp 1
.top_for:
# condition
load ra .stored_i
jmp.r.z .after_for
sub ra 1
store ra .stored_i

# body
load.sp ra 0
load.sp rb 1
xor ra rb
store.rp ra 0

# increment
add sp 1
add rp 1

jmp.r .top_for
.after_for:



add sp 3
ret



#int print(int* input); {
#    for (int i = 0; i < SIZE; i = i + 1) {
#		__print_num(input[i]);
#		write_vga_char(' ');
#
#        //std::cout << input[i];
#    }
#	write_vga_char(' ');
#
#    //std::cout << std::endl;
#}

# print
# INPUT:
#   ra = (int*) input vector
.print:
#.export print
mov rp ra
loadi ra 0x00
loadi.h ra 0x01
push ra

# setup to print
.extern write_vga_char
loada .write_vga_char

.print_top_for:
# condition and increment
load.sp ra 0
jmp.r.z .print_after_for
sub ra 1
store.sp ra 0

# body
load.rp ra 0
add rp 1
add ra 0x30
call .write_vga_char

jmp.r .print_top_for
.print_after_for:

add sp 1
ret
