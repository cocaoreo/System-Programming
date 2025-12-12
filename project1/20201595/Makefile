CC=gcc
TARGET=testlib
CFLAGS=-g -Wall
OBJECTS= main.o bitmap.o list.o debug.o hash.o hex_dump.o 
HEADER= main.h bitmap.h list.h debug.h hash.h hex_dump.h limits.h round.h

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

$(OBJECTS): $(HEADER)




.PHONY: clean

clean:
	rm $(OBJECTS)
	rm $(TARGET)
