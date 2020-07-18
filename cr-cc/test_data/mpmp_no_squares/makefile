CC=cr-cc
SRCS=mpmp_no_squares.c
OBJS=$(subst .c,.o,$(SRCS))
TTY=/dev/ttyS2

all: mpmp_no_squares.srec
	cat mpmp_no_squares.srec > $(TTY)

# This always gives an error even though it works
setup:
	-stty -F $(TTY) ospeed 115200

mpmp_no_squares.srec: $(OBJS) mpmp_no_squares_asm.o
	$(CC) -o mpmp_no_squares $^

mpmp_no_squares_asm.o: mpmp_no_squares_asm.s
	$(CC) -c -o $@ $<

$(OBJS): %.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm -f *.srec *.bin *.hex
	rm -f *.o
