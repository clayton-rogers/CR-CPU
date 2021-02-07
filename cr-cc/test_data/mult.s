.constant 0x8100 IO_OFFSET
.constant 0x8200 TIMER_OFFSET
.constant 0x8300 UART_OFFSET

.static 3 data 0xA1 0x07 0x33

.main:

loada .TIMER_OFFSET
load ra .TIMER_OFFSET
push ra

loadi rp, .data
loadi.h rp, .data
load.rp ra 0
load.rp rb 1
call.r ._mult
#call.r ._print_ra

loada .TIMER_OFFSET
load ra .TIMER_OFFSET
pop rb
sub ra rb
call.r ._print_ra

ret

# print_ra(ra)
#   prints the high byte, sleeps 4 seconds, 
#   and then prints the low byte
._print_ra:
push ra
	shftr ra 8
	call.r ._set_led
	loadi ra 0x00
	loadi.h ra 0x08
	call.r ._sleep_milli
	load.sp ra 0
	call.r ._set_led
pop ra
ret

# mult(ra, rb)
#   multiplies two numbers
#   takes up to 168 clocks
._mult:
push rp
# sp + 1 = return address
# sp + 0 = saved rp

    # first make sure smaller number is in ra, since it's faster
	loadi rp 0x00
#	push ra
#	push rb
#	sub ra rb
#	jmp.r.gz .mult_swap
#	jmp.r .mult_no_swap # no swap
#	.mult_swap:
#	pop ra
#	pop rb
#	jmp.r .mult_top
#	.mult_no_swap:
#	pop rb
#	pop ra
#	jmp.r .mult_top
	
	# loop over every bit of ra
	.mult_top:
	jmp.r.z .mult_exit
	push ra
	and ra 0x01
	jmp.r.z .mult_no_add
		add rp rb
	.mult_no_add:
	pop ra
	shftl rb 1
	shftr ra 1
	jmp.r .mult_top
	
.mult_exit:
mov ra rp	
pop rp
ret
#._mult:
#push rp
#	mov rp, ra
#	.mult_top:
#		jmp.r.z .mult_end
#		add rp, rb
#		sub ra, 1
#	jmp.r .mult_top
#
#	.mult_end:
#	mov ra, rp
#pop rp
#ret

# _set_led - sets the led states
# INPUT
#   ra - low 8 bits set leds
### VERIFIED ###
._set_led:
	loada    .IO_OFFSET[1]
	store ra .IO_OFFSET[1]
ret


# _sleep_micro - busy waits the given number of microseconds
# INPUT
#   ra - number of microseconds (max 8000)
### VERIFIED ###
._sleep_micro:
	jmp.r.z .sleep_micro_done
	shftl ra, 3 # multiply by 8
	sub ra, 6 # 1 call, 1 jmp, 1 shft, 1 sub, 2 ret
	# this will sleep for 16 cycles per loop
	.sleep_micro_top:
		sub ra, 1
	jmp.r.nz .sleep_micro_top
.sleep_micro_done:
ret

# _sleep_milli - busy waits for the given number of milliseconds
# INPUT
#   ra - number of milliseconds
### VERIFIED ###
._sleep_milli:
	push rb
	jmp.r.z .sleep_milli_done

	.sleep_milli_top:
	mov rb, ra
	loadi ra, 0xE8
	loadi.h ra, 0x03 # 1000 in ra
	call.r ._sleep_micro
	mov ra, rb
	sub ra, 1
	jmp.r.nz .sleep_milli_top

	.sleep_milli_done:
	pop rb
ret
