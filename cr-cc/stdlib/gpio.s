
.constant 0x8100 IO_OFFSET

.__set_gpio:
.export __set_gpio
loada    .IO_OFFSET[1]
store ra .IO_OFFSET[1]
ret
