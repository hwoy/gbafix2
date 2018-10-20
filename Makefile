CC=gcc

BIN=gba2

CFLAGS=-O2 -std=c90 -pedantic -Wall



.PHONY: all clean

all: $(BIN)

$(BIN): main.o function.o opt.o
	$(CC) -o $(BIN) main.o function.o opt.o

clean:
	rm -f *.o $(BIN) *.exe
function.o: function.c function.h
main.o: main.c function.h opt.h
opt.o: opt.c opt.h
