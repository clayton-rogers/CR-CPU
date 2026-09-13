

# return (bits & check_val) == check_val;

# check_bits
# INPUT:
#   ra = bits to check
#   rb = value to check
# OUTPUT:
#   ra = (bool) whether the bits are set
.check_bits:
.export check_bits

and ra rb
sub ra rb
jmp.r.z .are_equal
loadi ra 0
ret

.are_equal:
loadi ra 1
ret
