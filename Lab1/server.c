/*#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <arpa/inet.h>

#define MAX_COUNT_CLIENTS 100*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#define MAX_COUNT_CLIENTS 100
int clients[MAX_COUNT_CLIENTS];
char is_active[MAX_COUNT_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//Send data to the client

//void strict_write(int fd,char* buff,size_t n);
//receive data from client
void  variable_length_message_read(int fd, char* buff);
int strict_read(int fd,char* buff,size_t n);
void public(char *buffer, int self) {
    for (int count = 0; count < MAX_COUNT_CLIENTS; count ++ ) {

        if (count == self) 
           continue;
        if (is_active[count] == 0) 
           continue;

        int n = write(clients[count], buffer, strlen(buffer)); 

        if (n < -1) {
            perror("ERROR writing to socket");
            exit(0);
        }
    }
}

int strict_read(int fd,char* buff,size_t n)
{
   int p=0;
   while(n > 0)
   {
       int r = read(fd,buff+p,n-p);
       if(r <= 0){
          return 0;
       }
       p += r;
   }
   return 1;
}

void  variable_length_message_read(int fd, char* buff)
{
   size_t n = 0;
   char n2[5];
   strict_read(fd,n2,4);//get message length 
  
    n= atoi(n2);
   
   strict_read(fd,buff,n);

}



void *client_handler(void *arg) {

    pthread_mutex_lock(&mutex);

    char *id = (char *) arg;   //parameter transfer
    *id = 1;
    int socket_id = id - is_active;
    int sockfd = clients[socket_id];
    pthread_mutex_unlock(&mutex);
    if(sockfd<0){
        perror("ERROR on accept");
	exit(0);
		}
    while (1) {
        char buffer[255];
        bzero(buffer, 255);
	//uint32_t n=0;
        variable_length_message_read(sockfd, buffer);
        int read_bytes = read(sockfd, buffer, 254); 

        if (read_bytes < -1) {
            perror("ERROR on reading from socket");
            exit(0);
        }

        if (read_bytes == 0) {
            pthread_mutex_lock(&mutex);
            is_active[socket_id] = 0;
            close(sockfd);     
            pthread_mutex_unlock(&mutex);
            printf("%d quit \n", sockfd);

            break;

        }

        printf("receive the message : %s\n", buffer);
        fflush(stdout);
        public(buffer, socket_id);
    }
return NULL;
}
int main(int argc, char *argv[]) {
    
    unsigned int client_len;
    struct sockaddr_in server_address; //Server network address structure
    struct sockaddr_in client_address; //Client network address structure
    ssize_t n;
    int server_sockfd; //Server socket
    int newsockfd;

     /*create client socket */
   
    if (( server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < -1) {
        perror("ERROR on opening socket");
        return 1;
    }

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    uint16_t port_no = (uint16_t)atoi(argv[1]);
    server_address.sin_port = htons(port_no);

    /*bind the socket to the network address of the server*/
    if (bind(server_sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < -1) {
        perror("ERROR on binding");
        return 1;
    }

      /*Listen for connection requests */
    listen(server_sockfd, 5);
    client_len = sizeof(client_address);
    int cell = 0;
       
    // multithreading processes client's requests
    while (1) {
        newsockfd = accept(server_sockfd, (struct sockaddr *) &client_address, &client_len);
        clients[cell] = newsockfd;
        pthread_t tid;
        printf("Hi, accept client's connection  \n");
        pthread_create(&tid, NULL, client_handler, is_active + cell);  //create thread
        pthread_detach(tid);
        cell ++ ;
        if (cell >= MAX_COUNT_CLIENTS) {
            perror("error,the number of client is exceeded");
        }
    }
   
    pthread_mutex_destroy(&mutex);
    return 0;
}
