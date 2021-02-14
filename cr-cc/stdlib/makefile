CC=cr-cc
ASM_SRCS=mult.s div.s vga.s print_num.s timer.s gpio.s sleep.s
C_SRCS=math_sqrt.c
ASM_OBJS=$(subst .s,.o,$(ASM_SRCS))
C_OBJS=$(subst .c,.o,$(C_SRCS))
OBJS=$(ASM_OBJS) $(C_OBJS)

OS_SRCS=os.s

all: stdlib.a os.hex

os.hex: os.bin
	$(CC) --hex $<

os.bin: $(OS_SRCS)
	$(CC) --no-main --no-stdlib --link-addr 0x0000 $^

os.map: os.bin
	$(CC) --map $^

#stdlib.a: $(OBJS) os.map
stdlib.a: $(OBJS)
	$(CC) --lib -o $@ $^

$(ASM_OBJS): %.o: %.s
	$(CC) --no-stdlib -c -o $@ $<

$(C_OBJS): %.o: %.c
	$(CC) --no-stdlib -c -o $@ $<

clean:
	rm -f *.o *.a *.hex *.srec *.bin *.map
