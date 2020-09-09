all:server client
server:server.c
	clang -fsanitize=address -o server server.c 
client:client.c
	clang -fsanitize=address -o client client.c

clean:
	rm -f client server
