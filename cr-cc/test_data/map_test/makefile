# Change these
PROJECT=map
CFLAGS=
TTY=/dev/ttyS5

# Don't change these unless you know what you're doing
C_SRCS=
S_SRCS=main.s
C_OBJS=$(subst .c,.o,$(C_SRCS))
S_OBJS=$(subst .s,.o,$(S_SRCS))
OBJS=$(C_OBJS) $(S_OBJS)
CC=cr-cc


all: $(PROJECT).bin secret.bin

setup:
	$(info NOTE: This command always gives error but works)
	-stty -F $(TTY) ospeed 115200

upload: $(PROJECT).srec secret.srec
	cat secret.srec > $(TTY)
	cat $(PROJECT).srec > $(TTY)

sim: $(PROJECT).bin
	$(CC) --sim $<

clean:
	$(RM) -f *.o *.bin *.srec *.hex *.map

$(PROJECT).srec: $(PROJECT).bin
	$(CC) --srec $<

secret.srec: secret.bin
	$(CC) --srec $<

$(PROJECT).bin: $(OBJS) secret.map
	$(CC) -o $@ $(notdir $^)

secret.bin: secret.o
	$(CC) --link-addr 0x0600 --no-main -o $@ $^

secret.map: secret.bin
	$(CC) --map $^

secret.o: secret.s
	$(CC) $(CFLAGS) -c -o $@ $<

$(C_OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(S_OBJS): %.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

# see also possibly:
# https://codereview.stackexchange.com/questions/74136/makefile-that-places-object-files-into-an-alternate-directory-bin
