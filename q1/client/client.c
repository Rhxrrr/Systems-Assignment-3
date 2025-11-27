/*
 * Team Name: Hamzah Alhamadani, 
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

#define PORT 8000          // <-- Match the port your server runs on (e.g., ./server 8000)
#define BUFFER_SIZE 2048   // Bigger buffer for menu and benefit info

int main(void) {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    printf("Connecting to Benefits Canada Server...\n");

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return EXIT_FAILURE;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return EXIT_FAILURE;
    }

    // Connect
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Connected to server successfully!\n");

    // --- Step 1: Receive welcome message ---
    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        printf("Failed to receive welcome message\n");
        close(sock);
        return EXIT_FAILURE;
    }
    buffer[bytes_received] = '\0';
    printf("%s\n", buffer);

    // --- Step 2: Send name ---
    char name[100];
    printf("Please enter your name: ");
    if (!fgets(name, sizeof(name), stdin)) {
        printf("Failed to read name.\n");
        close(sock);
        return EXIT_FAILURE;
    }
    // Remove trailing newline
    name[strcspn(name, "\n")] = '\0';

    send(sock, name, strlen(name), 0);

    // --- Step 3: Receive greeting + initial menu ---
    bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        printf("Failed to receive greeting/menu\n");
        close(sock);
        return EXIT_FAILURE;
    }
    buffer[bytes_received] = '\0';
    printf("%s", buffer);  // menu already includes newlines

    // --- Main interaction loop ---
    while (1) {
        char choice[32];

        // IMPORTANT: Do NOT print "Enter your choice" here.
        // The server already sends "Enter your choice:" in the menu.
        if (!fgets(choice, sizeof(choice), stdin)) {
            printf("Input error.\n");
            break;
        }

        // Strip newline
        choice[strcspn(choice, "\n")] = '\0';

        // If user just presses enter, re-use loop and let server show menu again
        if (strlen(choice) == 0) {
            continue;
        }

        // Send choice (no extra newline required, but harmless if present)
        send(sock, choice, strlen(choice), 0);

        // --- Receive benefit info + y/n prompt in ONE go ---
        bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("Server closed the connection or failed to respond (benefit info).\n");
            break;
        }
        buffer[bytes_received] = '\0';
        printf("%s", buffer);   // includes benefit text + "Do you need any more information? (y/n): "

        // --- Read user's y/n response locally ---
        char again[8];
        if (!fgets(again, sizeof(again), stdin)) {
            printf("Input error.\n");
            break;
        }
        again[strcspn(again, "\n")] = '\0';

        // Send y/n to server
        send(sock, again, strlen(again), 0);

        // If NOT 'y', expect goodbye from server and exit
        if (strcasecmp(again, "y") != 0) {
            int bye = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (bye > 0) {
                buffer[bye] = '\0';
                printf("%s", buffer);
            }
            break;
        }

        // Otherwise, expect the refreshed menu
        int menu_recv = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (menu_recv <= 0) {
            printf("Server did not resend the menu.\n");
            break;
        }
        buffer[menu_recv] = '\0';
        printf("%s", buffer);
        // loop continues
    }

    close(sock);
    return 0;
}
