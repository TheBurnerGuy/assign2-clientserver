# Makefile Usage:
# make - makes client and server executables
# make clean - removes object files, temporary files, and executables

CC=gcc
CFLAGS=-m32 

mem_mod1: mem_mod1.o memlayout.o
	$(CC) mem_mod1.o memlayout.o $(CFLAGS) -o mem_mod1 
mem_mod2: mem_mod2.o memlayout.o
	$(CC) mem_mod2.o memlayout.o $(CFLAGS) -o mem_mod2
mem_mod3: mem_mod3.o memlayout.o
	$(CC) mem_mod3.o memlayout.o $(CFLAGS) -o mem_mod3

chat379: client.c 
	$(CC) client.c  $(CFLAGS) -o chat379
server379: server.c 
	$(CC) server.c  $(CFLAGS) -o server379

clean: 
	rm -f *~ chat379 server379
