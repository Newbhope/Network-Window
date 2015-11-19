#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>

int main (int argc, char *argv[]){
/*	int sock_fd=socket(AF_INET, SOCK_STREAM, 0);

	struct addrinfo addr, *result;
	memset(&addr, 0, sizeof(struct addrinfo));
	addr.ai_family=AF_INET;//ip4
	addr.ai_socktype=SOCK_STREAM;//tcp

	int error=getaddrinfo("www.illinois.edu", "80", &addr, &result);
	if(error!=0){
		fprintf(stderr, "error code: %s\n", gai_strerror(error));
		exit(1);
	}

	connect(sock_fd, result->ai_addr, result->ai_addrlen);


	char* buffer="GET / HTTP/1.0\r\n\r\n";
	printf("Sending:%s", buffer);
	printf("===\n");
	write(sock_fd, buffer, strlen(buffer));

	char response[1000];
	int length=read(sock_fd, response, 999);
	response[length]='\0';
	printf("%s\n", response);
*/	
	int saddr_size, data_size;
	struct sockaddr saddr;
	struct in_addr in;

	char* buffer=(char*) malloc(10000);

	FILE* log=fopen("log.txt", "w");
	if(log==NULL) exit(1);
	int sock=socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0){
		printf("Socket Error\n");
		exit(1);
	}
	while(1){
		saddr_size=sizeof saddr;
		data_size=recvfrom(sock, buffer, 10000, 0, &saddr, &saddr_size);
		if(data_size<0){
			printf("Recvfrom error\n");
			exit(1);
		}
	}
	close(sock);


	return 0;
}
