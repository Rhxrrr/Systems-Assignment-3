#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

void write_log(const char *message) {
    FILE *fp = fopen("log.txt", "a"); 
    if (fp == NULL) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char time_str[64];
    strftime(time_str, sizeof(time_str), "[%c]", t); 

    fprintf(fp, "%s %s\n", time_str, message);
    fclose(fp);
}

void *handle_client(void *arg) {
    int client_fd = *(int *) arg;
    free(arg);

    char buffer[2048];
    char name[100];
    char choice[10];
    
    // Send Welcome
    strcpy(buffer, "Welcome to Benefits Canada Server!\n\n");
    send(client_fd, buffer, strlen(buffer), 0);

    write_log("Welcome message sent!");

    // Receive Name
    int n = recv(client_fd, name, sizeof(name) - 1, 0);
    if(n <= 0) {
        close(client_fd);
        return NULL;
    }
    name[n] = '\0';
    name[strcspn(name, "\n")] = '\0';

    int first_run = 1;

    while(1) {
        // Send Menu
        snprintf(buffer, sizeof(buffer), 
                "\nHello %s\n"
                "To get information about benefits, please select from the followings:\n"
                "1. Canada Workers Benefits\n"
                "2. Canada Child Benefits\n"
                "3. Registered Disability Savings Plan\n"
                "4. Disability Tax Credit\n"
                "5. Canada Dental Benefits\n\n"  
                "Please enter your choice: ", name);
        if (send(client_fd, buffer, strlen(buffer), 0) < 0) break;
        
        if (!first_run) {
            write_log("Welcome message sent!");
        }
        write_log("Menu sent!");

        first_run = 0;

        // Receive choice
        n = recv(client_fd, choice, sizeof(choice) - 1, 0);
        if (n <= 0 || strcmp(choice, "QUIT") == 0) break;  

        choice[n] = '\0';
        choice[strcspn(choice, "\n")] = '\0';

        // 2. Open File and Search
        FILE *file = fopen("Benefits_Canada.txt", "r");
        char line[1024];
        int found = 0;
        char found_code[10];
        
        strcpy(buffer, "\nInvalid choice, try again.\n\n");

        if (file != NULL) {
            while (fgets(line, sizeof(line), file)) {
                // Make a copy of the line because strtok modifies it
                char line_copy[1024];
                strcpy(line_copy, line);

                // Parse: Serial;Code;Name;Details;URL;
                char *serial = strtok(line_copy, ";");
                char *code = strtok(NULL, ";");
                char *benefit_name = strtok(NULL, ";");
                char *details = strtok(NULL, ";");
                char *url = strtok(NULL, ";");

                if (serial != NULL && strcmp(serial, choice) == 0) {
                    snprintf(buffer, sizeof(buffer),
                            "\n------------------------------------------"
                            "\nPlease find below information about %s\n\n"
                            "%s (%s):\n"
                            "%s\n\n"
                            "Please visit the following link for more information:\n"
                            "%s"
                            "------------------------------------------\n\n", 
                            benefit_name, benefit_name, code, details, url);
                    if (code) strcpy(found_code, code);
                    found = 1;
                    break; 
                }
            }
            fclose(file);
        } else {
            printf("Error: Could not open Benefits_Canada.txt\n");
        }

        // 4. Send the Info to Client
        if (send(client_fd, buffer, strlen(buffer), 0) < 0) break;
        
        if (found) {
            printf("Requested information sent!\n");
            char log_msg[100];
            snprintf(log_msg, sizeof(log_msg), "%s information requested!", found_code);
            write_log(log_msg);
            write_log("Requested information sent!");
        }

        n = recv(client_fd, choice, sizeof(choice) - 1, 0);
        
        if (n <= 0 || strncmp(choice, "QUIT", 4) == 0) {
            break;
        }
    }
    close(client_fd);  
    return NULL;    
}

int main() {
    int server_fd, *pclient_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("ERROR: Server socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR: Server bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("ERROR: Server listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Loaded 5 benefits from file.\n");
    printf("Listening to incoming connection on port 8080...\n");

    while(1) {
        pclient_fd = malloc(sizeof(int));
        *pclient_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

        pthread_t t;
        pthread_create(&t, NULL, handle_client, pclient_fd);
        pthread_detach(t);
    }
    close(server_fd);
    return 0;
}