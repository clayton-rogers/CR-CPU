.constant 0x8400 VGA_OFFSET

# clear_screen
# INPUT
#   none
.constant 0x800 SCREEN_SIZE
.constant 0x20 SPACE
.clear_screen:
.export clear_screen
push ra
push rb
	# setup
	# rb space, ra counter
	loada .VGA_OFFSET
	loadi ra, .SCREEN_SIZE
	loadi.h ra, .SCREEN_SIZE
	loadi rb, .SPACE

	# clear every slot
	._cs_top:
		store rb, .VGA_OFFSET[0]
		sub ra, 1
		jmp.r.nz ._cs_top

	# reset cursor to begining
	loadi ra, 0
	store ra, .VGA_OFFSET[1]

pop rb
pop ra
ret

# write_vga_char
# INPUT
#   ra - character to write (0 .. 255)
.write_vga_char:
.export write_vga_char
	loada .VGA_OFFSET
	store ra, .VGA_OFFSET[0]
ret

# set_vga_cursor
# INPUT
#   ra - cursor position (0 .. 2048)
.set_vga_cursor:
.export set_vga_cursor
	loada .VGA_OFFSET
	store ra, .VGA_OFFSET[1]
ret
