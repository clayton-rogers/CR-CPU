
nop
nop

.add_numbers:
.export add_numbers
add ra, rb
ret

.main:
.export main
loadi ra, 10
loadi rb, 11
loada .add_numbers
call .add_numbers
ret
