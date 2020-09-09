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
#include <ctype.h>

#define PORT 53
void Name(char *name);

int main()
{   

    int socketfd;
    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    /* create a socket and bind it*/
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);
	
    if (bind(socketfd,(struct sockaddr*)&server_address,sizeof(server_address))<0){

        perror("failed");
        exit(1);
    }
    char *domain;
    char buf[256];
    socklen_t server_address_len = sizeof(server_address);

    // Set the getaddinfo structure
    struct addrinfo hint, *result, *current;
    
    char ss[INET_ADDRSTRLEN];
    
    memset(&hint, 0, sizeof(struct addrinfo));

    hint.ai_socktype = SOCK_STREAM;
    hint.ai_family = AF_INET;
    
    while (1)
    {
        bzero(&buf, 256);

        int queryLen = recvfrom(socketfd, buf, 255, 0, (struct sockaddr *)&server_address, &server_address_len);

        if(queryLen<0){
            perror("receive failed");
            exit(1);
        }

        char *name = (char*)(buf + sizeof(struct DNS_HEADER));
        struct DNS_HEADER *dns = (struct DNS_HEADER*)buf;
        dns->rcode = 3;
         dns->qr = 1;
        dns->rd = 1;
        dns->ra = 1;
 
        Name((char*)name);
        name++;
       //memcpy(domain, qname, strlen(qname));
        domain = name;
        struct QUESTION *question = (struct QUESTION*)(buf + queryLen - 4);
	    if (ntohs(question->qtype)!=1){
            sendto(socketfd, buf, queryLen, 0, (struct sockaddr *)&server_address, server_address_len);
            continue;
        }

        printf("\n Hi,Domain Name is: %s\n",domain);
        int ans_count, ret = getaddrinfo(domain, NULL,&hint,&result);
        
        if (ret < 0) {
            perror("failed\n");
            exit(1);
        }
        
        unsigned char* Answer = (unsigned char*)(buf + queryLen);
        struct R_DATA *data;
        int Pointer = 0x0cc0;
        for (current = result,ans_count=0 ; current != NULL; current = current->ai_next,ans_count++) {

            memcpy(Answer, &Pointer, 2);
            data = (struct R_DATA *)(Answer+2);
            data->type = htons(1);
            data->_class = htons(1);
            data->data_len = htons(4);

            memcpy(Answer + sizeof(struct R_DATA) + 2, &(((struct sockaddr_in *)(current->ai_addr))->sin_addr), 4);
        
            inet_ntop(AF_INET, Answer + sizeof(struct R_DATA) + 2, ss, sizeof(ss));

            printf("%s\n", ss);
            Answer = Answer + sizeof(struct R_DATA) + 6;
        }

        dns->ans_count = htons(ans_count);
        int result_len = queryLen + (16)*ans_count;
        sendto(socketfd, buf, result_len, 0, (struct sockaddr *)&server_address, server_address_len);
    }

    freeaddrinfo(result);
	
	return 0;
}

void Name(char *name) {
	int t = strlen(name);
	for (int i = 0; i < t; i++, name++) {
		if (!isalpha(*name)) {
			*name = '.';
		}
	}
       
}
