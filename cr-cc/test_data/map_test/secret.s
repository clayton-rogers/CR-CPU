
ret # loader jumps to the code so just jump back
.get_secret:
.export get_secret
loadi ra 0xCE
loadi.h ra 0xFA
ret
