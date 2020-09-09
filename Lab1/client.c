#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>

int foo = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



void strict_write(int sockfd, char* buf, uint32_t n) {
    int p = 0;
    while (p < n) {
        int r = write(sockfd, buf + p, n - p);
        if (r <= 0) {
            fprintf(stderr, "invalid write");
            pthread_exit(NULL);
        }
        p += r;
    }
}


void variable_length_message_write(int sockfd, char* buf, uint32_t n) {
    const uint32_t size = htonl(n);
    strict_write(sockfd, (char*)&size, sizeof(size));
    strict_write(sockfd, buf, n);
}

// the create thread is responsible for reading server feedback and pringting to standard output
void *server_handler(void *arg) {

      
    int *socket_fd = (int *) arg;

    while (1) {
        char buffer[255];
        
        int n = read(*socket_fd, buffer, 254);
         
        if (n == 0) {
            close(*socket_fd);
            exit(1);
        }

        buffer[n] = 0;
        pthread_mutex_lock(&mutex);

        if (foo) {
            printf("%s", buffer);
        }

        pthread_mutex_unlock(&mutex);
    }

}
int main(int argc, char *argv[]) {
    int socket_fd, n;
    struct sockaddr_in server_address;
    struct hostent *server;

    /* Error prompt */
    if (argc < 4) {
        fprintf(stderr, "usage %s localhost port name\n", argv[0]);
        exit(0);
    }

    

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &server_address.sin_addr.s_addr, (size_t) server->h_length);
    uint16_t port_no = (uint16_t) atoi(argv[2]);
    server_address.sin_port = htons(port_no);
   /*Bind the socket to the network address of the server*/
    if (connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("ERROR on connecting");
        exit(1);
    }

   
    pthread_t tid;
    pthread_create(&tid, NULL, server_handler, &socket_fd);//create thread
    pthread_detach(tid);

    
    // Process standard input and send it to the server
    while (1) {
        char buffer[256];
      
        bzero(buffer, 256);
       
        /* Get current time */
        time_t t = time(NULL);
        struct tm* lt = localtime(&t);
        int th = lt->tm_hour;
        int tm = lt->tm_min;
        char cur_time[BUFSIZ] = "";
        sprintf(cur_time,"%d:%d", th,tm);
        char *nick_name = argv[3];
    
        if (fgets(buffer, 255, stdin) != NULL) {
            if (*buffer == 'm' || *buffer == 'M') {
                pthread_mutex_lock(&mutex);
                foo = 0;
                pthread_mutex_unlock(&mutex);
                continue;
            }
 	    size_t buffer_len = strlen(buffer);
            size_t cur_time_len = strlen(cur_time);
            size_t nickname_len = strlen(nick_name);
            //printf("%ld\n", buffer_len);
            int message_len = buffer_len + cur_time_len + nickname_len + 5;
           // printf("%d\n", message_len);
            char *message = (char *) malloc(message_len);
            sprintf(message, "{%s}[%s]%s", cur_time, nick_name, buffer);  // print message
            pthread_mutex_lock(&mutex);
            variable_length_message_write(socket_fd,message, message_len);
            //send_data(socket_fd,message);
            foo = 1;
            pthread_mutex_unlock(&mutex);
             
        }
    }
    close(socket_fd);

    return 0;

}
