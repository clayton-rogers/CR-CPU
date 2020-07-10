
.extern __mult

.main:
.export main
loadi ra 2
loadi rb 3
loada .__mult
call .__mult
ret
