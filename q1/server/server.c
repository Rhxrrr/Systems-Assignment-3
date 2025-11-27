// server.c
/*
 * Benefits Canada Server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "client_handler.h"

#define DEFAULT_PORT 5000
#define BACKLOG 10  // max pending connections

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    if (argc >= 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        die("socket() failed");
    }

    // Allow immediate reuse of the port after server restart
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        die("setsockopt(SO_REUSEADDR) failed");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;   // listen on all interfaces
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        die("bind() failed");
    }

    if (listen(server_fd, BACKLOG) < 0) {
        die("listen() failed");
    }

    printf("=== Benefits Canada Server ===\n");
    printf("Listening on port %d...\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);
        if (client_fd < 0) {
            perror("accept() failed");
            continue; // don't exit the server, just skip this client
        }

        client_info_t *info = malloc(sizeof(client_info_t));
        if (!info) {
            fprintf(stderr, "malloc() failed\n");
            close(client_fd);
            continue;
        }

        info->client_fd = client_fd;
        info->client_addr = client_addr;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, info) != 0) {
            perror("pthread_create() failed");
            close(client_fd);
            free(info);
            continue;
        }
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
