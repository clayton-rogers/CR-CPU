
.static 64 bit_array
.constant 12 BIT_ARRAY_SIZE
# for some reason setting this to 52 or higher causes it to stop working...

.extern __mult
.extern print_num
.extern write_vga_char
.extern clear_screen

################################
# break
# breaks when the break_counter reaches zero
.static 1 break_counter 2
.break:
push ra
loada .break_counter
load ra .break_counter
sub ra 1
jmp.r.nz ._no_break
load.sp ra 0 # load back the called value before dying
halt # halt when counter == 0
._no_break:
store ra .break_counter
pop ra
ret

################################
# find_next_prime
# given the current offset in ra
# finds the next prime (unset) bit
.find_next_prime:
push ra
# sp+0 = current_pointer
.constant 0 current_pointer

._find_top:
load.sp ra .current_pointer
add ra 1
store.sp ra .current_pointer
call.r .get_bit
jmp.r.z ._find_after # if it's still zero this is a prime
jmp.r ._find_top
._find_after:
load.sp ra .current_pointer

add sp 1 # release pushed ra
ret

################################
# main
.main:
.export main
sub sp 5
# sp+4 =
# sp+3 =
# sp+2 =
# sp+1 = counter
# sp+0 = i
.constant 1 counter
.constant 0 i

loada .clear_screen
call .clear_screen

# i = 2
loadi ra 2
store.sp ra .i

# make sure to cross off 1 which is not a prime
# (this probably doesn't need to exist?)
loadi ra 1
loadi rb 1
call.r .set_bit


# while i * i < ARRAY_SIZE
._top:
	load.sp ra .i
	mov rb ra # copy
	loada .__mult
	call .__mult
	loadi rb .BIT_ARRAY_SIZE
	loadi.h rb .BIT_ARRAY_SIZE
	sub ra rb
	jmp.r.gz .after_loop

	# counter = i + i
	load.sp ra .i
	add ra ra
	store.sp ra .counter

	# while counter < ARRAY_SIZE
	._inner_top:
		load.sp ra .counter
		loadi rb .BIT_ARRAY_SIZE
		loadi.h rb .BIT_ARRAY_SIZE
		sub ra rb
		jmp.r.gz ._after_inner

		# set this bit
		load.sp ra .counter
		loadi rb 1
		call.r .set_bit

		# counter += i
		load.sp ra .counter
		load.sp rb .i
		add ra rb
		store.sp ra .counter

	jmp.r ._inner_top
	._after_inner:

	# find the next prime
	# i = get_next_prime()
	load.sp ra .i
	call.r .find_next_prime
	store.sp ra .i

	## TEMP BEGIN
	loadi ra 32 # space
	loada .write_vga_char
	call .write_vga_char
	call.r .output_primes
	## TEMP END

# end while
jmp.r ._top
.after_loop:


# now output all the primes we've found
loadi ra 32 # space
loada .write_vga_char
call .write_vga_char
loadi ra 0x41 # A for answer
loada .write_vga_char
call .write_vga_char
loadi ra 32 # space
loada .write_vga_char
call .write_vga_char
call.r .output_primes

add sp 5
ret

######################################################
# output_primes - outputs all the values that
# have not (yet) been marked as composite.
.output_primes:
push ra
push rb

# for i = 2; i < BIT_ARRAY_SIZE; ++i
loadi ra 2
store.sp ra .i
._top_output:
	# if i is a prime output it
	load.sp ra .i
	call.r .get_bit
	jmp.r.nz ._no_output
	load.sp ra .i
	loada .print_num
	call .print_num
	loadi ra 32 # space
	loada .write_vga_char
	call .write_vga_char
	._no_output:


	# increment i
	load.sp ra .i
	add ra 1
	store.sp ra .i
	# if i >= BIT_ARRAY_SIZE => exit
	loadi rb .BIT_ARRAY_SIZE
	loadi.h rb .BIT_ARRAY_SIZE
	sub ra rb
	jmp.r.gz ._after_output
jmp.r ._top_output
._after_output:

pop rb
pop ra
ret

######################################################
# get_bit - gets a given bit from the array
# INPUT
#   ra - the bit index
# OUTPUT
#   ra - the bit
.get_bit:
push ra
push rb
push rp
sub sp 3
# sp + 5 = bit_index
# sp + 4 = (saved rb)
# sp + 3 = (saved rp)
# sp + 2 =
# sp + 1 = subbyte_index (bit_index & 0x0F)
# sp + 0 = word
.constant 5 bit_index
.constant 1 subbyte_index
.constant 0 word

# calc and store subbyte_index
and ra 0x0F
store.sp ra .subbyte_index

# get value from array and store to stack
shftr ra 4
loadi rp .bit_array
loadi.h rp .bit_array
add rp ra
load.rp ra 0
store.sp ra .word

# create the bit mask
loadi ra 1
load.sp rb .subbyte_index
shftl ra rb

# mask the bit and ret
load.sp rb .word
and ra rb

# make sure ret bit is in zero spot
jmp.r.z ._get_bit_ret
loadi ra 1

._get_bit_ret:
add sp 3
pop rp
pop rb
add sp 1 # instead of pop ra
ret

######################################################
# set_bit - sets the given bit to the given value
# INPUT
#   ra - the bit index
#   rb - the value to set to (only LSB is used)
.set_bit:
push ra
push rb
push rp
sub sp 3
# sp + 5 = bit_index
# sp + 4 = bit_value
# sp + 3 = (saved rp)
# sp + 2 = mask
# sp + 1 = subbyte_index (bit_index & 0x0F)
# sp + 0 = word
.constant 5 bit_index
.constant 4 bit_value
.constant 2 mask
.constant 1 subbyte_index
.constant 0 word

# calc and store subbyte_index
and ra 0x0F
store.sp ra .subbyte_index

# get value from array and store to stack
shftr ra 4
loadi rp .bit_array
loadi.h rp .bit_array
add rp ra
load.rp ra 0
store.sp ra .word

# create the bit mask
loadi ra 1
load.sp rb .subbyte_index
shftl ra rb # mask now in ra
store.sp ra .mask

# mask the bit and ret
load.sp rb .word
load.sp ra .bit_value
jmp.r.z ._clear_bit
# else set bit
load.sp ra .mask
or ra rb
jmp.r ._after

._clear_bit:
load.sp ra .mask
push rb
loadi rb 0xFF
loadi.h rb 0xFF # no way to not a reg, so xor with 0xFFFF
xor ra rb
pop rb
and ra rb

# write word back again
._after:
store.rp ra 0

._set_bit_ret:
add sp 3
pop rp
pop rb
add sp 1 # instead of pop ra
ret
