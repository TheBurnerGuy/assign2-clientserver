# Makefile Usage:
# make : compile programs
# make client
# make server
# make clean



all:
	gcc -o chat379 client.c
	gcc -o server379 server.c

client:
	gcc -o chat379 client.c

server:
	gcc -o server379 server.c

clean:
	rm -rf chat379 server379 *~ *.log
