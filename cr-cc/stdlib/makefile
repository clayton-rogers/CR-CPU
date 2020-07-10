CC=cr-cc
SRCS=mult.s div.s vga.s
OBJS=$(subst .s,.o,$(SRCS))
all: stdlib.a

stdlib.a: $(OBJS)
	$(CC) --lib -o $@ $^

$(OBJS): %.o: %.s
	$(CC) -c -o $@ $<

clean:
	rm -f stdlib.a
	rm -f *.o
