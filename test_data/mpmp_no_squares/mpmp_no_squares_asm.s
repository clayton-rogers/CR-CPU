
.constant 4 WIDTH
.constant 16 SIZE
.static 16 grid


#int get_width();
.get_width:
.export get_width
loadi ra .WIDTH
loadi.h ra .WIDTH
ret

#int get_size();
.get_size:
.export get_size
loadi ra .SIZE
loadi.h ra .SIZE
ret

#int set_value(int pos, int value);
.set_value:
.export set_value
push rp
loadi rp .grid
loadi.h rp .grid
add rp ra
store.rp rb 0
pop rp
ret

#int get_value(int pos);
.get_value:
.export get_value
push rp
loadi rp .grid
loadi.h rp .grid
add rp ra
load.rp ra 0
pop rp
ret


# print_hex(ra) prints a single hex
# if the value is < 10 then add 48 to ascii char
# if the value is >= 10 then sub 10 and add 65 to ascii uppercase
.extern write_vga_char # from stdlib
.print_hex:
push ra
sub ra 10  # >= 10
jmp.r.gz ._is_letter
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

#int print_num(int num);
.print_num:
.export print_num
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

#int and(int,int);
.and:
.export and
and ra rb
ret
