// CLIENT - Unirc
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

#define MAXDATASIZE 100

pthread_mutex_t inp_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void *receive_message(void *args) {
    int sockfd = *(int *)args;
    char server_buffer[1024];

    while(1){
        memset(server_buffer, 0, sizeof(server_buffer));

        ssize_t bytes_received = recv(sockfd, server_buffer, sizeof(server_buffer) - 1, 0);
        if (bytes_received == -1) {
            // No data received, it's non-blocking, so this can happen
            continue;
        } else if (bytes_received == 0) {
            // Connection closed by the server
            printf("Server closed the connection\n");
            break;
        }

        server_buffer[strcspn(server_buffer, "\n")] = '\0';

        puts(server_buffer);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int sockfd, numbytes;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char servaddress[32];
    char port[32];
    char inp_buffer[1024], server_buffer[1024];
    char username[1024];
    char client_name[1024];

    pthread_t receive_thread;

    printf("Enter your username: ");
    fgets(client_name, sizeof(client_name), stdin);

    client_name[strcspn(client_name, "\n")] = '\0';

    printf("Enter server address: ");
    scanf("%s", servaddress);
        
    printf("Enter port number: ");
    scanf("%s", port);

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(servaddress, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    //inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    printf("Connected\n");

    freeaddrinfo(servinfo);

    strcat(client_name, ": ");
   
    pthread_create(&receive_thread, NULL, receive_message, (void *) &sockfd);

    while(1) {
        memset(inp_buffer, 0, sizeof(inp_buffer));
        memset(username, 0, sizeof(username));

        strcpy(username, client_name);
        
        int n = 0;

        fgets(inp_buffer, sizeof(inp_buffer), stdin);

        //inp_buffer[strcspn(inp_buffer, "\n")] = 0;

        if (strncmp(inp_buffer, "exit", 4) == 0) {
            printf("exiting...\n");
            close(sockfd);
            return 1;
        }
 
        strcat(username, inp_buffer);
 
        if (send(sockfd, username, strlen(username), 0) == -1) {
            perror("send");
            continue;
        }   

    }

    close(sockfd);

    return 0;
}
