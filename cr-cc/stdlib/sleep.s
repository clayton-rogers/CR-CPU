

# usleep - busy waits the given number of microseconds
# INPUT
#   ra - number of microseconds (max 8000)
### VERIFIED ###
.usleep:
.export usleep
	jmp.r.z .sleep_micro_done
	shftl ra, 3 # multiply by 8
	sub ra, 6 # 1 call, 1 jmp, 1 shft, 1 sub, 2 ret
	# this will sleep for 16 cycles per loop
	.sleep_micro_top:
		sub ra, 1
	jmp.r.nz .sleep_micro_top
.sleep_micro_done:
ret

# msleep - busy waits for the given number of milliseconds
# INPUT
#   ra - number of milliseconds
### VERIFIED ###
.msleep:
.export msleep
	push rb
	jmp.r.z .sleep_milli_done

	.sleep_milli_top:
	mov rb, ra
	loadi ra, 0xE8
	loadi.h ra, 0x03 # 1000 in ra
	call.r .usleep
	mov ra, rb
	sub ra, 1
	jmp.r.nz .sleep_milli_top

	.sleep_milli_done:
	pop rb
ret


# sleep - busy waits for the given number of seconds
# INPUT
#   ra - number of seconds
.sleep:
.export sleep
push rb
jmp.r.z .sleep_done


.sleep_top:
mov rb ra
loadi ra 0xE8
loadi.h ra 0x03 # 1000 in ra
call.r .msleep
mov ra rb
sub ra 1
jmp.r.nz .sleep_top


.sleep_done:
pop rb
ret