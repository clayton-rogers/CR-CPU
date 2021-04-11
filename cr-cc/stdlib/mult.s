

# mult(ra, rb)
#   multiplies two numbers
#   takes up to 168 clocks
.__mult:
.export __mult
push rp
# sp + 1 = return address
# sp + 0 = saved rp

    # first make sure smaller number is in ra, since it's faster
	loadi rp 0x00

	push ra
	push rb
	sub ra rb
	jmp.r.ge .mult_swap
	.mult_no_swap:
	pop rb
	pop ra
	jmp.r .mult_top
	.mult_swap:
	pop ra
	pop rb
	jmp.r .mult_top

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

