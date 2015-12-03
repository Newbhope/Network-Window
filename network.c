#include <ctype.h>
#include <signal.h>
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
//no parameters, output to stdout
//1 paramenter, log file to write out to
int main (int argc, char *argv[]){
	if(argc>2){
		printf("Run with no args or with the name of a log file\n");
		return 0;
	}
	int saddr_size;
	int t_data_size, u_data_size;
	struct sockaddr saddr;
	int t_count=1;
	int u_count=1;
	int go=0;//decides if writing to a file
	FILE* log;
	int stop=0;

	if(argv[1]!=NULL){
		log=fopen(argv[1], "w");
		if(log==NULL){
			perror(NULL);
			exit(1);
		}
		go=1;
	}
	int t_sock=socket(AF_INET, SOCK_RAW, IPPROTO_TCP);//tcp
	int u_sock=socket(AF_INET, SOCK_RAW, IPPROTO_UDP);//udp
	//int sock=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));//receives all
	if(t_sock<0 || u_sock<0){
		perror(NULL);
		exit(1);
	}

	pid_t tcp, udp;
	tcp=fork();
	if(tcp==0){//tcp handler
		char* buffer=(char*) malloc(10000);
		while(stop==0){
			saddr_size=sizeof saddr;
			t_data_size=recvfrom(t_sock, buffer, 10000, 0, &saddr, &saddr_size);
			if(t_data_size<0){
				perror(NULL);
				exit(1);
			}
			if(go==0)printf("TCP %d, size %d\n", t_count, t_data_size);
			else fprintf(log, "TCP %d, size %d\n", t_count, t_data_size);
			t_count++;
			//struct iphdr* iph=(struct iphdr*) buffer;
			//struct iphdr* iph=(struct iphdr*) (buffer+ sizeof(struct ethhdr));//use to receive all
			//printf("%d\n", iph->protocol);//6 = tcp, 17 = udp
		}
		free(buffer);
		return 0;
	}
	udp=fork();
	if(udp==0){//udp handler
		char* buffer=(char*) malloc(10000);
		while(stop==0){
			u_data_size=recvfrom(u_sock, buffer, 10000, 0, &saddr, &saddr_size);
			if(u_data_size<0){
				perror(NULL);
				exit(1);
			}
			if(go==0)printf("UDP %d, size %d\n", u_count, u_data_size);
			else fprintf(log, "TCP %d, size %d\n", t_count, t_data_size);
			u_count++;
		}
		free(buffer);
		return 0;
	}

	//parent
	char* line=NULL;
	size_t len=0;
	while(getline(&line, &len, stdin)!=-1){
		//printf("%s\n", line);
		if(strcmp(line, line)==0){//man this is stupid
			//kill(tcp, SIGKILL);
			//kill(udp, SIGKILL);
			stop=1;
			break;
		}
	}
	free(line);

	waitpid(tcp, NULL, NULL);
	waitpid(udp, NULL, NULL);
	fclose(log);
	close(t_sock);
	close(u_sock);


	return 0;
}
