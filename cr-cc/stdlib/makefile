CC=cr-cc
ASM_SRCS=mult.s div.s vga.s print_num.s timer.s
C_SRCS=math_sqrt.c
ASM_OBJS=$(subst .s,.o,$(ASM_SRCS))
C_OBJS=$(subst .c,.o,$(C_SRCS))
OBJS=$(ASM_OBJS) $(C_OBJS)

OS_MAIN=os.s
OS_SRCS=os-sleep.s os-uart.s os-gpio.s os-load-srec.s os-puts.s
OS_OBJS=$(subst .s,.o,$(OS_SRCS))
OS_MAIN_OBJS=os.o

all: stdlib.a os.hex os.map

os.hex: os.bin
	$(CC) --hex $<

os.bin: $(OS_MAIN_OBJS) $(OS_OBJS)
	$(CC) -v --no-main --no-stdlib --link-addr 0x0000 -o $@ $^

os.map: os.bin
	$(CC) --map $^

stdlib.a: $(OBJS)
	$(CC) --lib -o $@ $^

$(OS_OBJS): %.o: %.s
	$(CC) --no-stdlib -c -o $@ $<

$(OS_MAIN_OBJS): %.o: %.s
	$(CC) --no-stdlib -c -o $@ $<

$(ASM_OBJS): %.o: %.s
	$(CC) --no-stdlib -c -o $@ $<

$(C_OBJS): %.o: %.c
	$(CC) --no-stdlib -c -o $@ $<

clean:
	rm -f *.o *.a *.hex *.srec *.bin *.map
