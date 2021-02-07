######################################################
# sort - sorts the input registers largest to smallest
# INPUT
#    ra, rb, rp - three numbers
.sort:

# sort first two
push ra
sub ra, rb
jmp.r.gz .L1
# swap ra rb
mov ra, rb
pop rb
jmp.r .L3
.L1:
pop ra
.L3:

# sort second two
push ra
mov ra rb
mov rb rp
	push ra
	sub ra, rb
	jmp.r.gz .L2
	# swap ra rb
	mov ra, rb
	pop rb
	jmp.r .L4
	.L2:
	pop ra
	.L4:
mov rp rb
mov rb ra
pop ra

# sort first two again
push ra
sub ra, rb
jmp.r.gz .L5
# swap ra rb
mov ra, rb
pop rb
jmp.r .L6
.L5:
pop ra
.L6:

ret

######################################################
# calc_next - given three numbers, calculates the next iteration
# INPUT
#   ra, rb, rp - three numbers
.calc_next:
push ra
push rb
push rp
sub sp 3
# sp + 5 = ra (largest)
# sp + 4 = rb
# sp + 3 = rp (smallest)
# sp + 2 = new ra
# sp + 1 = new rb
# sp + 0 = new rp
sub ra rb
store.sp ra 2

sub rb rp
store.sp rb 1
load.sp ra 5

sub ra rp
mov rp ra

load.sp rb 1
load.sp ra 2

add sp, 6
ret

######################################################
# sum - sums the three numbers
# INPUT
#   ra, rb, rp
# OUTPUT
#   ra - the sum
.sum:
add ra rb
add ra rp
ret


######################################################
# mpmp_triangle - calculates whether a given set of three numbers
#                 is a solution to the mpmp problem
# INPUT
#  ra, rb - two numbers
#  sp + 1 - third number
# OUTPUT
#   ra - boolean
.mpmp_triangle:
.export mpmp_triangle
push rp
# sp + 2 - third number
# sp + 1 - ret address
# sp + 0 - saved rp
load.sp rp 2 # internal expects three numbers already loaded
call.r .mpmp_triangle_internal
# result in ra
pop rp
ret


######################################################
# mpmp_triangle_internal - calculates whether a given set of three numbers
#                          is a solution to the mpmp problem
# INPUT
#   ra, rb, rp - the three numbers
# OUTPUT
#   ra - boolean, true when success
.constant 100 COUNT # >= 1
.static 100 sums
.constant 14 REQUIRED_SUM
.mpmp_triangle_internal:
sub sp, 2 # temp sum and loop count
call.r .sort
push ra
push rb
push rp
# sp + 4 = temp sum
# sp + 3 = loop count
# sp + 2 = ra (largest)
# sp + 1 = rb
# sp + 0 = rp (smallest)

# init
loadi ra 0
store.sp ra 4 # temp sum
loadi ra .COUNT
store.sp ra 3 # loop count
load.sp rp 0
load.sp rb 1
load.sp ra 2

# for (i = COUNT; i != 0; --i) {
._main_loop_top:
call.r .calc_next
call.r .sort

# store numbers to calc count
store.sp rp 0
store.sp rb 1
store.sp ra 2
call.r .sum
store.sp ra 4 # calculate sum and store for later

load.sp ra 3  # if i == 0 goto end
sub ra 1
jmp.r.z ._after_loop
store.sp ra 3 # store i back
loadi rp .sums
loadi.h rp .sums
add rp ra # get address of sums[i]
load.sp ra 4 # load temp sum
store.rp ra 0

# restore numbers for next loop
load.sp rp 0
load.sp rb 1
load.sp ra 2

# } // end for
jmp.r ._main_loop_top


._after_loop:
# check if there was success i.e. the last 10 were 14
# sp + 3 = loop count
# int i = 10
loadi ra 0
store.sp ra 4
loadi ra 10
store.sp ra 3
# for (i = 10; i != 0; --i) {
._check_loop:
load.sp ra 3
sub ra 1
store.sp ra 3
jmp.r.z ._end_good

# ra == i
loadi rp .sums
loadi.h rp .sums
add rp ra # rp == &sums[i]
load.rp ra rp # ra = sums[i]

# if sum[i] != REQUIRED_SUM goto bad
loadi rb .REQUIRED_SUM
sub ra rb
jmp.r.nz ._end_bad

# } // end for
jmp.r ._check_loop

._end_good:
loadi ra 1
jmp.r ._end
._end_bad:
loadi ra 0
._end:
add sp, 5
ret

######################################################
.will_fit_in_4_bits:
push rb
loadi rb 0x0F
and rb ra # rb = ra & 0x0F
sub ra rb
jmp.r.nz ._will_not_fit
loadi ra 1
jmp.r ._will_fit_end
._will_not_fit:
loadi ra 0
._will_fit_end:
pop rb
ret

######################################################
# mpmp_pack_ans - packs the three result numbers into single int, 4 bits/num
# INPUT
#   ra, rb, rp
# OUTPUT
#   ra
.mpmp_pack_ans:
.export mpmp_pack_ans
push rb
push ra
loadi ra 0
push ra
# sp + 4 = rp
# sp + 3 = ret address
# sp + 2 = rb
# sp + 1 = ra
# sp + 0 = temp

load.sp ra 1
call.r .will_fit_in_4_bits
jmp.r.z ._pack_fail
load.sp ra 2
call.r .will_fit_in_4_bits
jmp.r.z ._pack_fail
load.sp ra 4
call.r .will_fit_in_4_bits
jmp.r.z ._pack_fail

load.sp ra 1
load.sp rb 2
shftl ra 4
or ra rb
load.sp rb 4
shftl ra 4
or ra rb

jmp.r ._pack_end
._pack_fail:
loadi ra 0xFF
loadi.h ra 0xFF

._pack_end:
add sp 3
ret
