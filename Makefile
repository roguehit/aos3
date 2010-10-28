CFLAGS = -I/home/rohit/aos3/xmlrpc_build/include -L/home/rohit/aos3/xmlrpc_build/lib -lxmlrpc -lxmlrpc_xmlparse -lxmlrpc_util -lxmlrpc_server -lxmlrpc_server_abyss -lxmlrpc_abyss -lxmlrpc_client -lxmlrpc_xmltok
all: server client

server: server.c
	gcc server.c $(CFLAGS) -o server

client: client.c 
	gcc client.c $(CFLAGS) -o client

phony: clean
clean:
	@rm client server
	@echo Cleaned
