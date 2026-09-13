
.constant 0x8200 timer_offset

# int get_timer_upper();
.get_timer_upper:
.export get_timer_upper
loada .timer_offset[1]
load ra .timer_offset[1]
ret

#int get_timer_lower();
.get_timer_lower:
.export get_timer_lower
loada .timer_offset[0]
load ra .timer_offset[0]
ret
