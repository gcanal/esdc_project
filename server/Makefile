all: server client

server: server.o game.o
	gcc -std=gnu99 -pthread -o server server.o game.o
client: client.o game.o
	gcc -std=gnu99 -pthread -o client client.o game.o
server.o: server.c game.h
	gcc -std=gnu99 -pthread -o server.o -c server.c
client.o: client_stub.c game.h
	gcc -std=gnu99 -pthread -o client.o -c  client_stub.c
game.o: game.c game.h
	gcc -std=gnu99 -o game.o -c game.c
test.o: game.h test.c
	gcc -std=gnu99 -o test.o -c test.c
test: game.o test.o
	gcc -std=gnu99 -o test game.o test.o
clean:
	rm -fr *.o server client test
