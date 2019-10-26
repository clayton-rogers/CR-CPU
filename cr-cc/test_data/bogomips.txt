# This file implementes the classic bogomips

.constant 0x8200 timer_addr
.constant 0x8101 output_addr
.constant 100 num_loops

loadi ra, .num_loops
loadi.h ra, .num_loops

loadi sp, 0x00
loadi.h sp, 0x01

# The actual testing part:
loada .timer_addr
loadi ra, 50

load rb, .timer_addr # start timer
call.r .usleep
load ra, .timer_addr # stop timer

sub ra, rb # get time taken

loada .output_addr
store ra, .output_addr

halt

# usleep - sleeps for tiny amounts of time
#          sleep time in clocks is
#          2x + 5 where x is ra
# INPUT
#   ra - number of loops
# OUTPUT
#   none
.usleep:
sub ra, 1
jmp.nz.r .usleep
ret
