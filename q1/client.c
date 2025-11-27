#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("ERROR: Client socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8080);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Connection Failed");
        close(sock);
        return 1;
    }

    char buffer[2048];
    int n;

    n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        perror("Receive failed or server closed connection");
        close(sock);
        return 1;
    }
    buffer[n] = '\0';
    printf("%s", buffer);

    printf("Please enter your name: ");
    fgets(buffer, sizeof(buffer), stdin);
    send(sock, buffer, strlen(buffer), 0);

    int loop = 1;

    while(loop) {
        n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            perror("Receive failed or server closed connection");
            close(sock);
            return 1;
        }
        buffer[n] = '\0';
        printf("%s", buffer);

        // Send choice
        fgets(buffer, sizeof(buffer), stdin);
        send(sock, buffer, strlen(buffer), 0);

        // Receive benefit info
        n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            perror("Receive failed or server closed connection");
            close(sock);
            return 1;
        }
        buffer[n] = '\0';
        printf("%s", buffer);

        printf("Do you need any more information? (y/n): ");

        while(1) {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                loop = 0;
                break;
            };
            buffer[strcspn(buffer, "\n")] = '\0';
            if (buffer[0] == 'n' || buffer[0] == 'N') {
                send(sock, "QUIT", 4, 0); 
                loop = 0;
                break;
            }
            else if (buffer[0] == 'y' || buffer[0] == 'Y') {
                send(sock, "OK", 2, 0);
                break;
            }
            else printf("\nInvalid input! Please enter 'y' or 'n'.\n");
        }
    }    

    close(sock);
    return 0;
}