
# int mult_two(int);
.mult_two:
.export mult_two
shftl ra 1
ret


################################
# break
# breaks when the break_counter reaches zero
#.static 1 break_counter 1
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



#int calc_number_of_arrangements(int num_member) {
#	int number_arrangements = 0;
#
#	for (int i = 1; i <= num_member/2; i = i + 1) {
#		for (int j = i; j <= num_member; j = j + 1) {
#			if (i * j == num_member) {
#				//output_soln(i, j);
#				number_arrangements = number_arrangements + 1;
#			}
#		}
#	}
#
#	return mult_two(number_arrangements);
#}

.extern __mult

# int calc_number_of_arrangements_asm(int num_member);
.calc_number_of_arrangements_asm:
.export calc_number_of_arrangements_asm
push ra
push rb
push rp
sub sp 5
#sp + 7 = saved ra (num_member)
#sp + 6 = saved rb
#sp + 5 = saved rp
#sp + 4 =
#sp + 3 =
#sp + 2 =
#sp + 1 = i
#sp + 0 = number_arrangements
.constant 7 num_member
.constant 1 i
.constant 0 number_arrangements
loadi ra 0
store.sp ra .number_arrangements

.static 1 break_counter 1

# use ra for temp
# use rb for temp
# use rp for j
loadi ra 1
store.sp ra .i
.outer_top:

	# j = i
	load.sp rp .i
	.inner_top:
		load.sp ra .i
		mov rb rp
		loada .__mult
		call .__mult
		# ra == i * j

		# if i*j == num_member then inc nunumber_arrangements
		load.sp rb .num_member
		sub ra rb
		jmp.r.nz ._no_increment
		load.sp ra .number_arrangements
		add ra 1
		store.sp ra .number_arrangements
		._no_increment:

		# inner for end condition
		add rp 1
		mov rb rp
		load.sp ra .num_member
		sub ra rb
	jmp.r.gz .inner_top
	.inner_end:

	# for end condition
	load.sp rb .i
	add rb 1
	store.sp rb .i
	load.sp ra .num_member
	shftr ra 1 # div num_member by 2
	sub ra rb
jmp.r.gz .outer_top
.outer_end:

# load the result and double
load.sp ra .number_arrangements
shftl ra 1 # double

add sp 5
pop rp
pop rb
add sp 1 # instead of pop ra
ret


.static 65 arrangements
.constant 65 NUM_ARRANGEMENTS

# int get_max_arrangements();
.get_max_arrangements:
.export get_max_arrangements
loadi ra .NUM_ARRANGEMENTS
loadi.h ra .NUM_ARRANGEMENTS
ret

# int set_soln(int num_arrangements, int num_members);
.set_soln:
.export set_soln
push ra
push rb
push rp

loadi rp .arrangements
loadi.h rp .arrangements
add rp ra
load.rp ra 0
jmp.r.nz ._dont_update
store.rp rb 0
._dont_update:

pop rp
add sp 2 # don't need to restore either reg
ret

# int get_soln(int num_arrangements);
.get_soln:
.export get_soln
push rp

loadi rp .arrangements
loadi.h rp .arrangements
add rp ra
load.rp ra 0

pop rp
ret
