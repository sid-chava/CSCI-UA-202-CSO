CC = gcc
CFLAGS = -arch x86_64

all:	assignment2

log.o:	log.s
	$(CC) $(CFLAGS) -c log.s

assignment2:	assignment2.o log.o
	$(CC) $(CFLAGS) -o assignment2 assignment2.o log.o
