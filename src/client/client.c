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

#include "../../include/server_utils.h"
#include "../../include/client_utils.h"

pthread_mutex_t inp_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char servaddress[32];
    char port[32];
    char inp_buffer[1024];
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
