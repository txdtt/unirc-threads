// SERVER - Unirc
#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>

#include "../../include/server_utils.h"

int main() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my ip
    
    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {

        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "SERVER FAILED TO BIND\n");
        exit(1);
    }

    if ((listen(sockfd, BACKLOG)) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server waiting for connection... \n");

    while(1) {
        client_data[thread_id].server_sockfd = sockfd;

        client_data[thread_id].sin_size = sizeof(client_data[thread_id].client_addr);

        if ((client_data[thread_id].client_sockfd = accept(client_data[thread_id].server_sockfd, (struct sockaddr *)&client_data[thread_id].client_addr, &client_data[thread_id].sin_size)) == -1 || thread_id == 5) {
            perror("accept");
            close(client_data[thread_id].client_sockfd);
            printf("server is full\n");
            continue;
        } else {
            printf("Accepted connection on socket %d\n", client_data[thread_id].client_sockfd);
        }

        struct client_data *thread_client_data = malloc(sizeof(struct client_data));

        if (thread_client_data == NULL) {
            perror("malloc");
            exit(1);
        }

        memcpy(thread_client_data, &client_data[thread_id], sizeof(struct client_data));

        thread_client_data->client_id = thread_id;

        pthread_create(&thread_array[thread_id], NULL, client_thread, (void *)thread_client_data);

        thread_id++;
    }

    return 0;
}
