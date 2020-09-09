
#include "dns_protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 53

int ff(char *name,char *qname); 
int Length(char *buffer, char *name);

int main(int argc, char** argv )
{
	int socketfd;
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	char dns_addr[40],domain_name[40],buf[256];
        memset(buf,0,256);
       if(argc < 3){
           fprintf(stderr, "usage: %s DNS_server Domain\n",argv[0]);
           exit(0);
        }
     
	strcpy(dns_addr,argv[1]);
	strcpy(domain_name,argv[2]);
	

        socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, dns_addr, &server_addr.sin_addr);

	struct DNS_HEADER *dns = (struct DNS_HEADER*)buf;
       
	unsigned char *qname = (unsigned char*)(buf + sizeof(struct DNS_HEADER));
        int domain_len = Length(buf,domain_name);
	socklen_t server_addr_len = sizeof(server_addr);
	int query_len = sendto(socketfd, buf, 12 + domain_len + 4, 0, (struct sockaddr *)&server_addr, server_addr_len );
	if (query_len <0){
		perror("sent failed");
        return 1;
	}

	memset(buf,0,256);
	if (recvfrom(socketfd, buf, 255, 0, (struct sockaddr *)&server_addr, &server_addr_len)<0){
		perror(" received failed");
        return 1;
	}
    
	struct R_DATA *r_data;
        char *result= buf + query_len;

	char str[INET_ADDRSTRLEN];
	dns = (struct DNS_HEADER*)buf;
	int ans_count = ntohs(dns->ans_count);

	printf("Domain : %s\n",argv[2]);
	while(ans_count > 0){
        r_data = (struct R_DATA *)(result+2);
        if (ntohs(r_data->type)==1){  
            printf("ip address : %s\n",
                            inet_ntop(AF_INET,result + 2 + sizeof(struct R_DATA), str, sizeof(str)));
        }
        result = result + sizeof(struct R_DATA) + ntohs(r_data->data_len) + 2 ;
		ans_count--;
    }

	close(socketfd);
	return 0;
}

int ff(char *name,char *qname)  
{  
       int len;
    int i =strlen(name)-1;  
    int j =0;
    int m = i+1;  
    qname[i+2] = 0;  
    for (; i>=0;i--,m--)  {  
        if (name[i] == '.')  {  
            qname[m] = j;  
            j=0;  
        }  
        else{  
            qname[m] = name[i];  
            j++;  
        }  
    }  
    qname[m] = j;  
   len =strlen(qname)+1;
    return len;
} 
int Length(char *buffer, char *name) {
	
	struct DNS_HEADER *dns = (struct DNS_HEADER*)buffer;
	dns->rd = 1;
       dns->id = htons(6);
        dns->q_count = htons(1);
	unsigned char *qname = (unsigned char*)(buffer + sizeof(struct DNS_HEADER));
	int namelen = ff(name, (char*)qname);
struct QUESTION *question = (struct QUESTION *)(qname + namelen);
    question-> qtype =  htons(1);
    question-> qclass = htons(1);
	
	return namelen;}

