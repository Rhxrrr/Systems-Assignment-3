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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 9001
#define BUFFER 1024

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // connect

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        return 1;
    }

    printf("Connected to server.\n"); 
    printf("You can type TIME, DATE, or EXIT.\n"); 

    while (1) {
        printf("> ");

        fgets(buffer, BUFFER, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        send(sock, buffer, strlen(buffer), 0); // send to server

        if (strcmp(buffer, "EXIT") == 0) {
            int bytes = recv(sock, buffer, BUFFER - 1, 0);
            buffer[bytes] = '\0';
            printf("Server: %s\n", buffer);
            break;
        }

        int bytes = recv(sock, buffer, BUFFER - 1, 0); // receive response from server
        buffer[bytes] = '\0';
        printf("Server: %s\n", buffer);
    }

    close(sock);
    return 0;
}

