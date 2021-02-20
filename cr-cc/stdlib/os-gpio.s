.constant 0x8100 IO_OFFSET

# _set_led - sets the led states
# INPUT
#   ra - low 8 bits set leds
### VERIFIED ###
.__set_led:
.export __set_led
	loada    .IO_OFFSET[1]
	store ra .IO_OFFSET[1]
ret
