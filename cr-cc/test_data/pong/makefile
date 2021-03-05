# Change these
PROJECT=pong
CFLAGS=
TTY=/dev/ttyS5

# Don't change these unless you know what you're doing
C_SRCS=$(shell find . -name "*.c")
S_SRCS=$(shell find . -name "*.s")
C_OBJS=$(subst .c,.o,$(C_SRCS))
S_OBJS=$(subst .s,.o,$(S_SRCS))
OBJS=$(C_OBJS) $(S_OBJS)
CC=cr-cc


all: $(PROJECT).bin

setup:
	$(info NOTE: This command always gives error but works)
	-stty -F $(TTY) ospeed 115200

upload: $(PROJECT).srec
	cat $< > $(TTY)

sim: $(PROJECT).bin
	$(CC) --sim $<

clean:
	$(RM) *.o *.bin *.srec *.hex *.map

$(PROJECT).srec: $(PROJECT).bin
	$(CC) --srec $<

$(PROJECT).bin: $(OBJS)
	$(CC) -v -o $@ $(notdir $^)

$(C_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(S_OBJS): %.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

# see also possibly:
# https://codereview.stackexchange.com/questions/74136/makefile-that-places-object-files-into-an-alternate-directory-bin
