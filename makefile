CC=gcc
CFLAGS=-Wall -Wextra -g

all: client server

client: client/cli

client/cli: client/client.c client/client.h
	$(CC) $(CFLAGS) -o client/cli client/client.c

server: server/serveur

server/serveur: server/Puissance4_server.c server/Puissance4_server.h utils/replay.c utils/replay.h
	$(CC) $(CFLAGS) -o server/serveur server/Puissance4_server.c utils/replay.c

clean:
	rm -f client/cli server/serveur utils/*.o

rebuild: clean all
