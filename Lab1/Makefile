all:server client
server:server.c
	clang -fsanitize=address -g -o server server.c 
client:client.c
	clang -fsanitize=address -g -o client client.c 

clean:
	rm -f client server
