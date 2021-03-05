
# From:
# https://stackoverflow.com/questions/3062746/special-simple-random-number-generator
# https://en.wikipedia.org/wiki/Xorshift


#/* The state word must be initialized to non-zero */
#uint32_t xorshift32(struct xorshift32_state *state)
#{
#	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
#	uint32_t x = state->a;
#	x ^= x << 13;
#	x ^= x >> 17;
#	x ^= x << 5;
#	return state->a = x;
#}

#// returns a random number between 0 and 3
#int get_rand() {
#	static long state;
#	long x = state;
#	x = x ^ (x << 13);
#	x = x ^ (x >> 17);
#	x = x ^ (x << 5);
#	state = x;
#   return x & 0x03;
#}

# approx 278 clocks per number

.static 2 state 0xCAFE 0xBEEF
.get_rand:
.export get_rand
push rb
push rp # shouldn't be required....
sub sp 4

# sp offsets:
# 6 ret address
# 5 saved rp
# 4 saved
# 3 scratch 2
# 2 scratch 1
# 1 x msb
# 0 x lsb

.constant 1 high
.constant 0 low
.constant 3 sc_high
.constant 2 sc_low

# rp will be a pointer to state
loadi rp .state
loadi.h rp .state

# store state to x
load.rp ra 0
load.rp rb 1
store.sp ra 0
store.sp rb 1

#	x = x ^ (x << 13);
load.sp ra .high
shftl ra 13
load.sp rb .low
shftr rb 3
or ra rb
store.sp ra .sc_high

load.sp ra .low
shftl ra 13
store.sp ra .sc_low

load.sp ra .low
load.sp rb .sc_low
xor ra rb
store.sp ra .low

load.sp ra .high
load.sp rb .sc_high
xor ra rb
store.sp ra .high

#	x = x ^ (x >> 17);
load.sp ra .high
shftr ra 1
store.sp ra .sc_low
loadi ra 0
store.sp ra .sc_high

load.sp ra .low
load.sp rb .sc_low
xor ra rb
store.sp ra .low

load.sp ra .high
load.sp rb .sc_high
xor ra rb
store.sp ra .high


#	x = x ^ (x << 5);
load.sp ra .high
shftl ra 5
load.sp rb .low
shftr rb 11
or ra rb
store.sp ra .sc_high

load.sp ra .low
shftl ra 5
store.sp ra .sc_low

load.sp ra .low
load.sp rb .sc_low
xor ra rb
store.sp ra .low

load.sp ra .high
load.sp rb .sc_high
xor ra rb
store.sp ra .high


#	state = x;
load.sp ra 0
load.sp rb 1
store.rp ra 0
store.rp rb 1

#.extern __print_num
#.extern write_vga_char
#loada .__print_num
#call .__print_num
#loadi ra 0x20
#loada .write_vga_char
#call .write_vga_char
#mov ra rb
#loada .__print_num
#call .__print_num
#   return x & 0x03;
and ra 0x03


add sp 4
pop rp
pop rb
ret
