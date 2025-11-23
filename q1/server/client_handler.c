// client_handler.c
#include "client_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define WELCOME_MSG "Welcome to Benefits Canada Server!\n"

void *client_thread(void *arg) {
    client_info_t *info = (client_info_t *)arg;
    int client_fd = info->client_fd;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(info->client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(info->client_addr.sin_port);

    printf("[+] New client connected from %s:%d (fd=%d)\n",
           client_ip, client_port, client_fd);

    // Step 1 requirement: send welcome message
    ssize_t sent = send(client_fd, WELCOME_MSG, strlen(WELCOME_MSG), 0);
    if (sent < 0) {
        perror("send() failed");
        close(client_fd);
        free(info);
        return NULL;
    }

    // For Step 1, we don’t need to handle further communication yet.
    // But we can keep the connection open and just read/discard whatever client sends.
    char buffer[256];
    ssize_t n;
    while ((n = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("[Client %s:%d] %s\n", client_ip, client_port, buffer);
        // Later steps: parse and respond here.
    }

    if (n == 0) {
        printf("[-] Client %s:%d disconnected.\n", client_ip, client_port);
    } else if (n < 0) {
        perror("recv() failed");
    }

    close(client_fd);
    free(info);
    return NULL;
}
