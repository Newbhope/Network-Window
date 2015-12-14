#all: net client server


net: network.o
	cc -o  net network.o
network.o: network.c
	cc -c -std=c99 network.c

#client: client.o
#	cc -o client client.o
#client.o: client.c
#	cc -c client.c

#server: server.o
#	cc -o server server.o
#sever.o: server.c
#	cc -c server.c

clean:
	rm net network.o #client client.o server server.o
