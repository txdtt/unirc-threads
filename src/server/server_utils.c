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

struct client_data client_data[5];

char inp_buffer[1024];

pthread_mutex_t inp_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

struct client_data client_data[5];

pthread_t thread_array[5];

int thread_id = 0;

void *relay_message(int thread_id_local) {

    for (int i = 0; i < thread_id; i++) {
        if (i != thread_id_local && client_data[i].client_sockfd != -1 && client_data[i].client_sockfd != 0) {
            ssize_t sent_bytes = send(client_data[i].client_sockfd, inp_buffer, strlen(inp_buffer), 0);
            if (sent_bytes == -1) {
                perror("send");
                printf("Error sending to client %d\n", i);
            } else {
                printf("Sent %zd bytes to client %d: %s\n", sent_bytes, i, inp_buffer);
            }
        }
    }

    return NULL;

}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    } 

    return &(((struct sockaddr_in *)sa)->sin_addr);
}

void *client_thread(void *args) {
    struct client_data *client_data_thread = (struct client_data *)args;    

    printf("THREAD ID %ld CREATED\n", (long)pthread_self());
    
    //printf("THREAD ID %d CLIENT_ID\n", thread_id_local);

    printf("CLIENT SOCKET FD %d\n", client_data_thread->client_sockfd);

    printf("SERVER SOCKET FD %d\n", client_data_thread->server_sockfd);

    while(1) {
        int thread_id_local = client_data_thread->client_id;

        memset(inp_buffer, 0, sizeof(inp_buffer));

        ssize_t bytes_received = recv(client_data_thread->client_sockfd, inp_buffer, sizeof(inp_buffer), 0);       
        if (bytes_received == -1) {
            perror("recv");
            break;
        } else if (bytes_received == 0 || strncmp(inp_buffer, "exit", 4) == 0) {
            printf("Client disconnected\n");
            close(client_data_thread->client_sockfd);
            /*for (int i = thread_id_local; i <= thread_id; i++) {
                client_data[i].client_id -= 1;
                thread_array[i] -= 1;
                thread_id--;
            }*/
            pthread_exit(&thread_array[thread_id_local]);
            return NULL;
        }

        printf("thread_id %d: %s\n", thread_id_local, inp_buffer);

        pthread_mutex_lock(&inp_buffer_mutex);

        relay_message(thread_id_local);

        pthread_mutex_unlock(&inp_buffer_mutex);
    }

    return NULL;
}
