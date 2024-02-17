CC=gcc
CFLAGS = -arch x86_64

all:	assign3

bin.o:	bin.s
	$(CC) $(CFLAGS) -c bin.s

assign3:	assignment3.o binary_search_tree.o bin.o
	$(CC) $(CFLAGS) -o assign3 assignment3.o binary_search_tree.o bin.o
 



