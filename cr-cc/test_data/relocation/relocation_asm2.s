
.constant 4 WIDTH
.constant 16 SIZE
.static 16 grid


#int get_width();
.get_width:
.export get_width
loadi ra .WIDTH
loadi.h ra .WIDTH
ret

#int get_size();
.get_size:
.export get_size
loadi ra .SIZE
loadi.h ra .SIZE
ret

#int set_value(int pos, int value);
.set_value:
.export set_value
push rp
loadi rp .grid
loadi.h rp .grid
add rp ra
store.rp rb 0
pop rp
ret

#int get_value(int pos);
.get_value:
.export get_value
push rp
loadi rp .grid
loadi.h rp .grid
add rp ra
load.rp ra 0
pop rp
ret
