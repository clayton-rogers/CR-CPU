


#int data[100];
#
#int* get_array() {
#	return data;
#}
#
#int get_array_size() {
#	return 100;
#}

.static 100 data

.get_array:
.export get_array
loadi ra, .data
loadi.h ra, .data
ret

.get_array_size:
.export get_array_size
loadi ra, 100
ret
