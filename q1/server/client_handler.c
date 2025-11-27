// client_handler.c
#include "client_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

#define WELCOME_MSG "Welcome to Benefits Canada Server!\n"

static const char *MAIN_MENU =
"------------------------------\n"
" Benefits Canada Main Menu\n"
"------------------------------\n"
"1. Employment Insurance (EI)\n"
"2. Canada Pension Plan (CPP)\n"
"3. Old Age Security (OAS)\n"
"4. Child Care Benefit (CCB)\n"
"5. Disability Benefit\n"
"------------------------------\n"
"Enter your choice: ";

// Read a benefit line from the file given a serial number
void read_benefit_from_file(int serial, char *out_buf, size_t buf_size) {
    FILE *fp = fopen("../data/Benefits_Canada.txt", "r");
    if (!fp) {
        snprintf(out_buf, buf_size,
                 "ERROR: Could not open Benefits_Canada.txt\n");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char copy[1024];
        strncpy(copy, line, sizeof(copy));
        copy[sizeof(copy) - 1] = '\0';

        char *token = strtok(copy, ";");
        if (!token) continue;

        int file_serial = atoi(token);
        if (file_serial != serial) continue;

        char *name = strtok(NULL, ";");
        char *desc = strtok(NULL, ";");
        char *elig = strtok(NULL, ";");

        if (!name || !desc || !elig) {
            snprintf(out_buf, buf_size,
                     "Data format error for option %d.\n", serial);
            fclose(fp);
            return;
        }

        // Trim trailing newlines/spaces from fields if needed
        name[strcspn(name, "\r\n")] = '\0';
        desc[strcspn(desc, "\r\n")] = '\0';
        elig[strcspn(elig, "\r\n")] = '\0';

        snprintf(out_buf, buf_size,
                 "---------------------------------------\n"
                 "Benefit Name: %s\n"
                 "Description : %s\n"
                 "Eligibility : %s\n"
                 "---------------------------------------\n\n",
                 name, desc, elig);

        fclose(fp);
        return;
    }

    snprintf(out_buf, buf_size,
             "No benefit found for option %d.\n", serial);
    fclose(fp);
}


// Main client thread function
void *client_thread(void *arg) {
    client_info_t *info = (client_info_t *)arg;
    int client_fd = info->client_fd;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(info->client_addr.sin_addr),
              client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(info->client_addr.sin_port);

    printf("[+] New client connected from %s:%d (fd=%d)\n",
           client_ip, client_port, client_fd);

    char buffer[2048];
    char client_name[256];

    // Step 1: Send welcome
    send(client_fd, WELCOME_MSG, strlen(WELCOME_MSG), 0);

    // Step 2: Receive client name
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        printf("[-] Client disconnected before sending name.\n");
        close(client_fd);
        free(info);
        return NULL;
    }
    buffer[n] = '\0';
    buffer[strcspn(buffer, "\r\n")] = '\0'; // strip newline

    strncpy(client_name, buffer, sizeof(client_name));
    client_name[sizeof(client_name) - 1] = '\0';

    // Step 3: Send greeting + menu
    char greeting_menu[4096];
    snprintf(greeting_menu, sizeof(greeting_menu),
             "Hello %s! Welcome!\n%s", client_name, MAIN_MENU);
    send(client_fd, greeting_menu, strlen(greeting_menu), 0);

    // =================== MAIN LOOP ===================
    while (1) {
        // Receive choice
        n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            printf("[-] Client %s:%d disconnected while sending choice.\n",
                   client_ip, client_port);
            break;
        }
        buffer[n] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0'; // strip newline

        int choice = atoi(buffer);
        printf("[SERVER] Client %s:%d selected benefit: %d\n",
               client_ip, client_port, choice);

        // Get benefit info
        char benefit_info[4096];
        read_benefit_from_file(choice, benefit_info, sizeof(benefit_info));

        // Send benefit info + y/n prompt in ONE go
        char response[8192];
        snprintf(response, sizeof(response),
                 "%sDo you need any more information? (y/n): ",
                 benefit_info);
        send(client_fd, response, strlen(response), 0);

        // Receive y/n
        n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            printf("[-] Client %s:%d disconnected while answering y/n.\n",
                   client_ip, client_port);
            break;
        }
        buffer[n] = '\0';
        buffer[strcspn(buffer, "\r\n")] = '\0'; // strip newline

        // Look at first non-space character
        char *p = buffer;
        while (*p == ' ' || *p == '\t') p++;

        if (*p == 'y' || *p == 'Y') {
            // send menu again
            send(client_fd, MAIN_MENU, strlen(MAIN_MENU), 0);
            continue;
        } else {
            const char *bye =
                "Thank you for using Benefits Canada. Goodbye!\n";
            send(client_fd, bye, strlen(bye), 0);
            break;
        }
    }

    printf("[*] Closing connection to %s:%d (fd=%d)\n",
           client_ip, client_port, client_fd);

    close(client_fd);
    free(info);
    return NULL;
}
