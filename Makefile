all: cleanup.out client.out server.out

cleanup.out: cleanup.c
	gcc -o cleanup.out cleanup.c

client.out: client.c
	gcc -o client.out client.c

server.out: server.c
	gcc -o server.out server.c

clean:
	rm -f cleanup.out client.out server.out
