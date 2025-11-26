// client_handler.c
#include "client_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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



void read_benefit_from_file(int serial, char *out_buf, size_t buf_size) {
    FILE *fp = fopen("../data/Benefits_Canada.txt", "r");
    if (!fp) {
        snprintf(out_buf, buf_size, "ERROR: Could not open Benefits_Canada.txt\n");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        int file_serial = atoi(strtok(line, ";"));
        if (file_serial == serial) {
            char *name = strtok(NULL, ";");
            char *desc = strtok(NULL, ";");
            char *elig = strtok(NULL, ";");

            snprintf(out_buf, buf_size,
                     "---------------------------------------\n"
                     "Benefit Name: %s\n"
                     "Description : %s\n"
                     "Eligibility : %s\n"
                     "---------------------------------------\n",
                     name, desc, elig);

            fclose(fp);
            return;
        }
    }

    snprintf(out_buf, buf_size, "No benefit found for option %d.\n", serial);
    fclose(fp);
}


// step 1
void *client_thread(void *arg) {
    client_info_t *info = (client_info_t *)arg;
    int client_fd = info->client_fd;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(info->client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(info->client_addr.sin_port);

    printf("[+] New client connected from %s:%d (fd=%d)\n",
           client_ip, client_port, client_fd);

    
    send(client_fd, WELCOME_MSG, strlen(WELCOME_MSG), 0);

    char buffer[1024];

   
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        close(client_fd);
        free(info);
        return NULL;
    }
    buffer[n] = '\0';

    char client_name[256];
    strncpy(client_name, buffer, sizeof(client_name));

    
    char greeting_menu[2048];
    snprintf(greeting_menu, sizeof(greeting_menu),
             "Hello %s! Welcome!\n%s", client_name, MAIN_MENU);

    send(client_fd, greeting_menu, strlen(greeting_menu), 0);


// step 3
    n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        close(client_fd);
        free(info);
        return NULL;
    }
    buffer[n] = '\0';

    int choice = atoi(buffer);
    printf("[SERVER] Client selected benefit: %d\n", choice);

   
    char benefit_info[4096];
    read_benefit_from_file(choice, benefit_info, sizeof(benefit_info));

    
    send(client_fd, benefit_info, strlen(benefit_info), 0);

    printf("[SERVER] Requested Information Sent\n");

    close(client_fd);
    free(info);
    return NULL;
}
