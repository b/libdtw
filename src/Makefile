PROJECT = libdtw.a
CC=gcc
CFLAGS=-O2 -Wall
SOURCES=ucr_dtw.c deque.c
OBJECTS=ucr_dtw.o deque.o

all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	libtool -o $(PROJECT) -static $(OBJECTS)

clean:
	rm *.o libdtw.a