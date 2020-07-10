
# 10 / 3:
# /   |1010
# 0011|0000
# shift right until entire number is in the shifted dividend (add to count)
# /   |1010
# 0000|1100
# try to sub, if pass add one to result, then shift again
# /   |1010
# 0000|0110
# success
# res = 0001
# /   |0100
# 0000|0110
# shift and shift res
# res = 0010
# /   |0100
# 0000|0011
# success
# res = 0011
# /   |0001
# 0000|0011
# count has reached 4 so end. res = 0011 (3) and rem = 0001 (1)


# NOTE:
# dividend
# --------
# divisor

###########################################################
# rem(ra, rb)
#   divides two numbers and returns the remainder
#   NOTE: both numbers must be positive
.__rem:
.export __rem
call.r .__div
mov ra rb
ret

###########################################################
# div(ra, rb) -> ra/rb
#   divides two numbers
#   NOTE: both numbers must be positive
.__div:
.export __div
.constant 16 NUMBER_BITS
push rp # saved rp
push rb # divisor
push ra # dividend
loadi ra 0
push ra # shifted_divisor
push ra # result
loadi ra .NUMBER_BITS
push ra # count
loadi ra 0
loadi.h ra 0x80
push ra # mask = 0x8000


# relative to sp
.constant 6 STACK_SIZE
.constant 6 saved_rp
.constant 5 divisor
.constant 4 dividend
.constant 3 shifted_divisor
.constant 2 result
.constant 1 count
.constant 0 mask

# if divisor == 0 return 0
load.sp ra .divisor
jmp.r.nz .L1
halt # div by zero causes cpu to halt
.L1:

# shift right until whole number is in the shifted spot
# while (divisor != 0) {
#   shifted_divisor >>= 1;
#   count -= 1;
#   if (divisor & 0x01) {
#     shifted_divisor |= mask;
#   }
#   divisor >>= 1;
# }
.first_loop:
load.sp ra .divisor
jmp.r.z .after_first # break if dividend == 0
load.sp rb .shifted_divisor
shftr rb 1
# ra = divisor, rb = shifted_divisor
load.sp rp .count
sub rp 1
store.sp rp .count
loadi rp 0x01
and ra rp
# ra = divisor & 0x01, rb = shifted_divisor
jmp.r.z .after_if
load.sp rp .mask
or rb rp
.after_if:
# modified shifted_divisor near beginning of loop, so must store even if we don't mask
store.sp rb .shifted_divisor
load.sp ra .divisor
shftr ra 1
store.sp ra .divisor

jmp.r .first_loop
.after_first:

# for now do one more shift to deal with the lack of carry
# if this is going to stay here it can be optimized with the next line
load.sp ra .shifted_divisor
shftr ra 1
store.sp ra .shifted_divisor

# Perform subtraction on each
# divisor = shifted_divisor;
# while (count != 0) {
#   count--;
#   result <<= 1;
#   int temp = dividend - divisor;
#   if (temp >= 0) {
#     dividend = temp;
#     result++;
#   }
#   divisor >>= 1;
# }
# result -> ra
# dividend -> rb (rem)
load.sp ra .shifted_divisor
store.sp ra .divisor
.second_loop:
load.sp ra .count
jmp.r.z .after_second
sub ra 1
store.sp ra .count
load.sp ra .result
shftl ra 1
store.sp ra .result
load.sp ra .dividend
load.sp rb .divisor
sub ra rb
jmp.r.gz .inside_if
jmp.r .after_if_2
.inside_if:
store.sp ra .dividend
load.sp ra .result
add ra 1
store.sp ra .result
.after_if_2:
#load.sp rb .divisor
# divisor is still in rb
shftr rb 1
store.sp rb .divisor
jmp.r .second_loop
.after_second:

# return result in ra and remainder in rb
load.sp ra .result
load.sp rb .dividend

.div_exit:
add sp .STACK_SIZE
pop rp
ret
