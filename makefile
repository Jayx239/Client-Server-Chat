all:
	gcc -o client client.c -lrt -pthread
	gcc -o server server.c -lrt -pthread

