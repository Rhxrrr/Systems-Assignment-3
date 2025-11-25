/*
 * Team Name: Hamzah Alhamadani
 * Group Number: 24
 * Course: CSCI 3310U
 * Assignment: 3 - Benefits Canada Server/Client
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8000          // <-- Match the port your server runs on
#define BUFFER_SIZE 2048   // Bigger buffer for menu and benefit info

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    printf("Connecting to Benefits Canada Server...\n");

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    // Connect
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Connected to server successfully!\n");

    // Receive welcome message
    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        printf("Failed to receive welcome message\n");
        close(sock);
        return -1;
    }
    buffer[bytes_received] = '\0';
    printf("%s\n", buffer);

    // STEP 2: Send name and get menu
    char name[100];
    printf("Please enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0; // Remove newline

    send(sock, name, strlen(name), 0);

    // Receive greeting + menu
    bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        printf("Failed to receive greeting/menu\n");
        close(sock);
        return -1;
    }
    buffer[bytes_received] = '\0';
    printf("%s\n", buffer);

    // STEP 3: Send choice and get response
    char choice[10];
    printf("Enter your choice: ");
    fgets(choice, sizeof(choice), stdin);
    choice[strcspn(choice, "\n")] = 0;

    send(sock, choice, strlen(choice), 0);

    // Receive benefit information
    bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
    } else {
        printf("Failed to receive benefit information\n");
    }

    printf("\nPress Enter to exit...");
    getchar();

    close(sock);
    return 0;
}
