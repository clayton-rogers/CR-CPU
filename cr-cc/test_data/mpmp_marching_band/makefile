CC=cr-cc
SRCS=mpmp_march.c mpmp_alt.c
OBJS=$(subst .c,.o,$(SRCS))
TTY=/dev/ttyS2

all: mpmp_march.srec
	cat mpmp_march.srec > $(TTY)

# This always gives an error even though it works
setup:
	-stty -F $(TTY) ospeed 115200

mpmp_march.srec: mpmp_march.bin
	$(CC) --srec $<
mpmp_march.bin: $(OBJS) asm.o
	$(CC) -o mpmp_march.bin $^

asm.o: asm.s
	$(CC) -c -o $@ $<

$(OBJS): %.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm -f *.srec *.bin *.hex
	rm -f *.o
