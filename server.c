// SERVER - Unirc
// aaaa
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

int main() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
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
