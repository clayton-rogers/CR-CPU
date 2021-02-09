
.static 2900 data
.constant 2900 size

.extern __mult
.extern __print_num
.extern write_vga_char
.extern clear_screen

.extern print_as_dec


#################################################
# main
.main:
.export main
loada .clear_screen
call .clear_screen

# helps alignment on 80 char screen
loadi ra 0x20
loada .write_vga_char
call .write_vga_char

call.r .sieve_and_output
ret

#################################################
# set_bit
#   INPUT: ra - bit to set
.set_bit:
loadi rp .data
loadi.h rp .data
add rp ra
loadi ra 1
store.rp ra 0
ret

#################################################
# get_bit
#   INPUT: ra - bit to get_bit
.get_bit:
loadi rp .data
loadi.h rp .data
add rp ra
load.rp ra 0
ret

#################################################
# print - input from ra
# prints "0123 "
.print:
.export print
###loada .__print_num
###call .__print_num
loada .print_as_dec
call .print_as_dec
loadi ra 0x20
loada .write_vga_char
call .write_vga_char
ret

#################################################
# debug_print
#   INPUT:
#		[ra] = current_prime
#		[ra + 1] = i
# prints "sp0123 i0123 "

.debug_print:
mov rp ra

# if (debug_print_count == 0) return else --debug_print_count;
.static 1 debug_print_count 50
loada .debug_print_count
load ra .debug_print_count
jmp.r.nz .after_debug_count_check
ret
.after_debug_count_check:
sub ra 1
store ra .debug_print_count


loadi ra 0x63
loada .write_vga_char
call .write_vga_char
loadi ra 0x70
loada .write_vga_char
call .write_vga_char
load.rp ra 0
loada .__print_num
call .__print_num
loadi ra 0x20
loada .write_vga_char
call .write_vga_char
loadi ra 0x69
loada .write_vga_char
call .write_vga_char
load.rp ra 1
loada .__print_num
call .__print_num
loadi ra 0x20
loada .write_vga_char
call .write_vga_char

ret



#################################################
# sieve and output
.sieve_and_output:
sub sp 10
# sp + 10 = ret addr
# sp + 9 =
# sp + 8 =
# sp + 7 =
# sp + 6 =
# sp + 5 =
# sp + 4 =
# sp + 3 =
# sp + 2 =
# sp + 1 = i
# sp + 0 = current_prime
.constant 0 current_prime
.constant 1 i

loadi ra 2
store.sp ra .current_prime

.outer_while:

#########
# first_for prep
load.sp ra .current_prime
store.sp ra .i
.first_for:
# first_for end condition
	#mov ra sp
	#call.r .debug_print

load.sp ra .i
loadi rb .size
loadi.h rb .size
sub ra rb # if i - size is pos then break
jmp.r.gz .after_first_for
# first_for contents
load.sp ra .i
call.r .set_bit
# first_for increment
load.sp ra .i
load.sp rb .current_prime
add ra rb
store.sp ra .i
jmp.r .first_for
.after_first_for:
#########

#########
# print(current_prime)
load.sp ra .current_prime
call.r .print
#########

#########
load.sp ra .current_prime
add ra 1
store.sp ra .current_prime
.inner_while:
# while condition
load.sp ra .current_prime
call.r .get_bit
jmp.r.z .after_inner_while
# while body
load.sp ra .current_prime
add ra 1
store.sp ra .current_prime  # ++ current_prime
loadi rb .size
loadi.h rb .size
sub ra rb
jmp.r.z .after_outer_while # if current_prime == size : break
jmp.r .inner_while
.after_inner_while:
#########

jmp.r .outer_while
.after_outer_while:


add sp 10
ret
