all: 
	gcc -o client cchess-client.c -pthread
	gcc -o server cchess-server.c -pthread