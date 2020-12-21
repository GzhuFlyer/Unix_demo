.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=server client getlocalIp
all:$(BIN)
%.o:%.c
		$(CC) $(CFLAGS) -C $< -O $@
clean:
		rm -f *.o $(BIN)