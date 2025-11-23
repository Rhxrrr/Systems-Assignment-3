#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <netinet/in.h>

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} client_info_t;

void *client_thread(void *arg);

#endif // CLIENT_HANDLER_H
