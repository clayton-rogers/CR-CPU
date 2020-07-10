

# ra/rb -> result:ra, remainder:rb
.extern __div

.constant 0x7FFF dividend
.constant 0x0132 divisor

.main:
.export main
loadi ra .dividend
loadi.h ra .dividend
loadi rb .divisor
loadi.h rb .divisor
loada .__div
call .__div
ret
