CC=cr-cc
ASM_SRCS=mult.s div.s vga.s print_num.s timer.s
C_SRCS=math_sqrt.c
ASM_OBJS=$(subst .s,.o,$(ASM_SRCS))
C_OBJS=$(subst .c,.o,$(C_SRCS))
OBJS=$(ASM_OBJS) $(C_OBJS)

all: stdlib.a

stdlib.a: $(OBJS)
	$(CC) --lib -o $@ $^

$(ASM_OBJS): %.o: %.s
	$(CC) -c -o $@ $<

clean:
	rm -f stdlib.a
	rm -f *.o
