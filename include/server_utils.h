#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <pthread.h>
#include <sys/socket.h>

#define PORT "3940"
#define BACKLOG 10 

struct client_data {
    int client_id;
    int client_sockfd;
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    int server_sockfd;
    char client_name[128];
};

extern struct client_data client_data[5];

extern char inp_buffer[1024];

extern int thread_id;

extern pthread_mutex_t inp_buffer_mutex;

extern pthread_t thread_array[5];

void *relay_message(int thread_id_local);

void *get_in_addr(struct sockaddr *sa);

void *client_thread(void *args);

#endif
