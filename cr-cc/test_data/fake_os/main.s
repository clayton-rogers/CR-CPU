.extern user_load_srec
.extern clear_screen
.extern write_vga_char

.main:
.export main
loada .clear_screen
call .clear_screen
loadi ra 0x65
loada .write_vga_char
call .write_vga_char
loadi ra 0x65
loada .write_vga_char
call .write_vga_char
loadi ra 0x65
loada .write_vga_char
call .write_vga_char

loada .user_load_srec
call .user_load_srec

loadi rb .exit
loadi.h rb .exit
push rb # exit
push ra # srec load add
ret     # jump to user code


.exit:
ret
