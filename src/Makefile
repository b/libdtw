PROJECT = libdtw.a
CC=gcc
CFLAGS=-O2 -Wall
SOURCES=core.c deque.c
OBJECTS=core.o deque.o

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	libtool -o $(PROJECT) -static $(OBJECTS)

clean:
	rm *.o libdtw.a