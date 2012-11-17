CC=gcc
CFLAGS=-O2 -Wall
SOURCES=test.c
INCLUDES=-I../src
LDFLAGS=-L../src -ldtw

test: ../src/libdtw.a
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) $(SOURCES) -o test
	./test data.txt query.txt 128 0.05

clean:
	rm test