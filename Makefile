net: network.o
	cc -o net network.o
network.o: network.c
	cc -c network.c
clean:
	rm net main.o
