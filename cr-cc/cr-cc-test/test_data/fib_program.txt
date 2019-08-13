loadc RA, 1
loadc RB, 1
add rc, ra, rb
mov ra, rb
mov rb, rc
out ra
jmp 2