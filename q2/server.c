/*
 * Team Name: Hamzah Alhamadani, Fardin Alam, Mohammed Abdulaziz
 * Group Number: 24
 * Course: CSCI 3310U
 * Assignment: 3
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define PORT 9001
#define BUFFER 1024
//thread fcn
void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);

    char buffer[BUFFER];
    int bytes;

    while (1) {
        memset(buffer, 0, BUFFER);

        bytes = recv(client_sock, buffer, BUFFER - 1, 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';

        if (strcmp(buffer, "TIME") == 0) { // commands 
            time_t now = time(NULL);
            struct tm *t = localtime(&now);

            char time_str[100];
            sprintf(time_str, "%02d:%02d:%02d",
                    t->tm_hour, t->tm_min, t->tm_sec);

            send(client_sock, time_str, strlen(time_str), 0);
        }

        else if (strcmp(buffer, "DATE") == 0) {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);

            char date_str[100];
            sprintf(date_str, "%04d-%02d-%02d",
                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

            send(client_sock, date_str, strlen(date_str), 0);
        }

        else if (strcmp(buffer, "EXIT") == 0) {
            char msg[] = "Goodbye";
            send(client_sock, msg, strlen(msg), 0);
            close(client_sock);
            pthread_exit(NULL);
        }

        else {
            char msg[] = "Invalid command"; //invalid input
            send(client_sock, msg, strlen(msg), 0);
        }
    }

    close(client_sock);
    pthread_exit(NULL);
}

int main() {
    int server_fd, *client_sock;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0); // listening socket

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address)); // bind and listen
    listen(server_fd, 10);

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_sock = malloc(sizeof(int));
        *client_sock = accept(server_fd, (struct sockaddr*)&address, &addrlen); // accept client in loop

        pthread_t tid; // spawn thread for client
        pthread_create(&tid, NULL, handle_client, client_sock);
        pthread_detach(tid);
    }

    return 0;
}

