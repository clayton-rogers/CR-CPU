#################################################
# print_hex(ra) prints a single hex
# if the value is < 10 then add 48 to ascii char
# if the value is >= 10 then sub 10 and add 65 to ascii uppercase
.extern write_vga_char # from stdlib
.print_hex:
push ra
sub ra 10  # >= 10
jmp.r.ge ._is_letter
load.sp ra 0
add ra 48
jmp.r ._print_after
._is_letter:
load.sp ra 0
add ra 55
._print_after:
loada .write_vga_char
call .write_vga_char
add sp 1
ret

#################################################
#int print_num(int num);
.__print_num:
.export __print_num
push ra
shftr ra 12
call.r .print_hex
load.sp ra 0
shftr ra 8
and ra 0x0F
call.r .print_hex
load.sp ra 0
shftr ra 4
and ra 0x0F
call.r .print_hex
load.sp ra 0
and ra 0x0F
call.r .print_hex
add sp 1
ret
