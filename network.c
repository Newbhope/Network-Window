#define _GNU_SOURCE
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
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
#include <netinet/tcp.h>
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
		unlink(argv[1]);
		log=fopen(argv[1], "a");
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

	struct sockaddr_in source;

	pid_t tcp, udp;
//	int t_pipe[2];
//	pipe(t_pipe);

	tcp=fork();
	if(tcp==0){//tcp handler
		//close(t_pipe[0]); //close read end
		char buffer[10000];
		memset(buffer, 0, 10000);
		while(stop==0){
			saddr_size=sizeof saddr;
			t_data_size=recvfrom(t_sock, buffer, 10000, 0, &saddr, &saddr_size);
			if(t_data_size<0){
				perror(NULL);
				exit(1);
			}
			struct iphdr* iph=(struct iphdr*) buffer;
			unsigned short iphdrlen=iph->ihl*4;
			//printf("%d\n", iphdrlen);
			struct tcphdr *tcph=(struct tcphdr*) (buffer+iphdrlen);

			memset(&source, 0, sizeof(source));
			source.sin_addr.s_addr=iph->saddr;

			//source.sin_addr.s_addr=iph->daddr;

			//printf("%d\n", iph->protocol);//6 = tcp, 17 = udp


			printf("TCP segment from %s on port %d of size %d\n", inet_ntoa(source.sin_addr), ntohs(tcph->source), t_data_size);
			if(go==1){
				fprintf(log, "TCP segment from %s on port %d of size %d\n", inet_ntoa(source.sin_addr), ntohs(tcph->source), t_data_size);
				fflush(log);
			}
			t_count++;
		}
		return 0;
	}
	udp=fork();
	if(udp==0){//udp handler
		char buffer[10000];
		memset(buffer, 0, 10000);
		while(stop==0){
			u_data_size=recvfrom(u_sock, buffer, 10000, 0, &saddr, &saddr_size);
			if(u_data_size<0){
				perror(NULL);
				exit(1);
			}

			struct iphdr* iph=(struct iphdr*) buffer;
			memset(&source, 0, sizeof(source));
			source.sin_addr.s_addr=iph->saddr;
			unsigned short iphdrlen=iph->ihl*4;
			struct udphdr* udph=(struct udphdr*) buffer+iphdrlen;

			printf("UDP packet from %s on port %d of size %d\n", inet_ntoa(source.sin_addr), ntohs(udph->source), u_data_size);
			if(go==1){
				fprintf(log, "UDP packet from %s on port %d of size %d\n", inet_ntoa(source.sin_addr), ntohs(udph->source), u_data_size);
				fflush(log);
			}
			u_count++;
		}
		return 0;
	}

	//parent
	char* line=NULL;
	size_t len=0;
	while(getline(&line, &len, stdin)!=-1){
		//printf("%s\n", line);
		if(strcmp(line, line)==0){//man this is stupid
			kill(tcp, SIGKILL);
			kill(udp, SIGKILL);
			stop=1;
			break;
		}
	}
	free(line);

	waitpid(tcp, NULL, WUNTRACED);
	waitpid(udp, NULL, WUNTRACED);
	fclose(log);
	close(t_sock);
	close(u_sock);
	

if(go==1){//only print out cumulative data if a log was made
	char temp_addr[20];
	int temp_port;
	int temp_data;

	int size=30;

	int tcp_data=0;
	int t_count=0;//total tcp segments

	char* ipaddr[size];
	for(int i=0; i<size; i++){
		ipaddr[i]=calloc(size, 1);	
	}
	int t_ipsize=0;

	int addr_data[size];
	memset(&addr_data, 0, 4*size);

	int ports[size];
	memset(&ports, 0, sizeof(int)*size);
	int p_size=0;


	int udp_data=0;
	int u_count=0;

	char* u_ipaddr[size];
	for(int i=0; i<size; i++){
		u_ipaddr[i]=calloc(size, 1);	
	}
	int u_addr_data[size];
	memset(&u_addr_data, 0, 4*size);
	int u_ipsize=0;

	FILE* read=fopen(argv[1], "r");
	if(read==NULL){
		perror(NULL);
	}
	rewind(read);
	
	while(!feof(read)){
		if(fscanf(read, "TCP segment from %s on port %d of size %d\n", &temp_addr, &temp_port, &temp_data)!=0){
			tcp_data+=temp_data;

			//printf("%s %s\n", temp_addr, ipaddr[0]);
			
			for(int i=0; i<=t_ipsize; i++){
				if( strcmp(ipaddr[i], temp_addr)==0){//address in current spot of array is the same
					addr_data[i]+=temp_data;//add amount of data sent by specific address
					break;
				}
				if(i==t_ipsize){//add address to array and increment count by 1
					//printf("should only show up once\n");
					ipaddr[t_ipsize]=strdup(temp_addr);
					addr_data[t_ipsize]+=temp_data;
					t_ipsize++;
					break;
				}
			}

			for(int i=0; i<=p_size; i++){
				if(temp_port==ports[i]) break;
				if(i==p_size){
					ports[i]=temp_port;
					p_size++;
					break;
				}
			}


			t_count++;//total amount of tcp segments
		}
		else if(fscanf(read, "UDP packet from %s on port %d of size %d\n", &temp_addr, &temp_port, &temp_data)!=0){
			udp_data+=temp_data;

			for(int i=0; i<=u_ipsize; i++){
				if( strcmp(u_ipaddr[i], temp_addr)==0){//address in current spot of array is the same
					u_addr_data[i]+=temp_data;//add amount of data sent by specific address
					break;
				}
				if(i==u_ipsize){//add address to array and increment count by 1
					//printf("should only show up once\n");
					u_ipaddr[u_ipsize]=strdup(temp_addr);
					u_addr_data[u_ipsize]+=temp_data;
					u_ipsize++;
					break;
				}
			}

			for(int i=0; i<=p_size; i++){
				if(temp_port==ports[i]) break;
				if(i==p_size){
					ports[i]=temp_port;
					p_size++;
					break;
				}
			}

			u_count++;
		}

		else printf("fscanf failed\n");
		
	}

	//printf("%d\n", t_ipsize);
	//printf("%d\n", u_ipsize);


	printf("#######################################################################\n");
	printf("TCP info from session:\n");
	printf("Total amount of TCP data received: %d bytes\n", tcp_data);
	printf("Total segments received: %d\n", t_count);
	printf("IP addresses connected: \n");
	for(int i=0; i<t_ipsize; i++){
		printf("%s sent %d bytes\n", ipaddr[i], addr_data[i]);
	}
	printf("\n");
	printf("UDP info from session:\n");
	printf("Total amount of UDP data received: %d bytes\n", udp_data);
	printf("Total packets received: %d\n", u_count);
	printf("IP addresses connected: \n");
	for(int i=0; i<u_ipsize; i++){
		printf("%s sent %d bytes\n", u_ipaddr[i], u_addr_data[i]);
	}
	printf("\n");
	printf("Ports used during the session:\n");
	for(int i=0; i<p_size; i++){
		printf("%d ", ports[i]);
	}
	printf("\n\n");


	for(int i=0; i<t_ipsize; i++){
		free(ipaddr[i]);
	}
	for(int i=0; i<u_ipsize; i++){
		free(u_ipaddr[i]);
	}

}

	return 0;
}
