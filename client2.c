#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define SERVER_PORT 5566
#define MAX_BUFFER_SIZE 4096
#define SERVER_IP "127.0.0.1"
#define MAX_CLIENT_PATH_LENGTH 1024

void removeLastNewline(char *str) {
    size_t len = strlen(str);

    // Check if the string is not empty
    if (len > 0) {
        // Find the last occurrence of '\n'
        char *lastNewline = strrchr(str, '\n');

        // If '\n' is found, replace it with null terminator
        if (lastNewline != NULL) {
            *lastNewline = '\0';
        }
    }
}


int main() 
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[4096];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Main loop for continuously taking input and sending to the server
    while (1) 
    { 
        // Get user input
        char band[MAX_BUFFER_SIZE];
        printf("Enter a message to send to the server (type 'exit' to quit): ");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);

        removeLastNewline(buffer);

        strcpy(band,buffer);
        // Send data to the server
        if (send(sock, buffer, sizeof(buffer), 0) == -1) {
            perror("Sending data to server failed");
            break;
        }


           // Set up timeout variables
        fd_set readfds;
        struct timeval timeout;
        timeout.tv_sec = 5;  // 5 seconds timeout

        // Wait for data to be ready to read with timeout
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);
        

        if (activity == 0) {
            // Timeout occurred
            printf("Timeout: No response from the server within 5 seconds.\n");
            close(sock);
            exit(EXIT_FAILURE);
        } else if (activity < 0) 
        {
            perror("Select error");
            close(sock);
            exit(EXIT_FAILURE);
        }

        printf("ENtered\n");

        if(strncmp(buffer,"READ",4)==0 || strncmp(buffer,"WRITE",5)==0 || strncmp(buffer,"GETINFO",7)==0)
        {
            // printf("ENtered\n");
            char buffer1[MAX_BUFFER_SIZE];
            memset(buffer1 , 0, sizeof(buffer1));
            // bzero(buffer1,sizeof(buffer1));
            if (recv(sock, buffer1, sizeof(buffer1), 0) == -1) 
            {   
                perror("Receiving data from server failed");
                close(sock);
                exit(EXIT_FAILURE);
            }
            // send(sock, &y, sizeof(y), 0);
            printf("values: %s\n",buffer1);

            char* token;
            int count = 0;

            token = strtok(buffer1, " ");
            int temp_port;

                while(token!=NULL)
                {
                    if(count==1)
                    {
                        temp_port=atoi(token);
                        printf("Updated: %d\n",temp_port);
                    }

                    token = strtok(NULL, " ");
                    count++;
                }  

            int client_sock = 0;
            struct sockaddr_in client_addr;
            // Create socket
            if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Socket creation failed");
                exit(EXIT_FAILURE);
            }

            client_addr.sin_family = AF_INET;
            client_addr.sin_port = htons(temp_port);

            // Convert IP address from string to binary form
            if (inet_pton(AF_INET, SERVER_IP, &client_addr.sin_addr) <= 0) {
                perror("Invalid address/ Address not supported");
                exit(EXIT_FAILURE);
            }

            // Connect to the server
            printf("port: %d\n",temp_port);
            if (connect(client_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
                perror("Connection failed");
                exit(EXIT_FAILURE);
            }
            
            if (send(client_sock, band, sizeof(band), 0) == -1) 
            {
                perror("Sending data to server failed");
                break;
            }

             bzero(buffer1,sizeof(buffer1));

            while(1)
            {
                recv(client_sock, buffer1, sizeof(buffer1),0);
                if(strcmp(buffer1,"STOP")==0)
                {
                    break;
                }
                printf("%s",buffer1);
                bzero(buffer1,sizeof(buffer1));
                strcpy(buffer1,"GOT");
                send(client_sock,buffer1,sizeof(buffer1),0);
            }

            printf("\n");
            printf("Yes: %s\n",buffer1);
            close(client_sock);

        }

        else if(strncmp(buffer,"CREATE",6)==0 || strncmp(buffer,"DELETE",6)==0)
        {
            char buffer1[MAX_BUFFER_SIZE];
            bzero(buffer1,sizeof(buffer1));
            if (recv(sock, buffer1, sizeof(buffer1), 0) == -1) 
            {
                perror("Receiving data from server failed");
                close(sock);
                exit(EXIT_FAILURE);
            }

            printf("Yes: %s\n",buffer1);
        }

        // else if(strncmp(buffer,"COPY",4)==0)
        // {
        //     char buffer1[MAX_BUFFER_SIZE];
        //     if (recv(sock, buffer1, sizeof(buffer1), 0) == -1) 
        //     {
        //         perror("Receiving data from server failed");
        //         close(sock);
        //         exit(EXIT_FAILURE);
        //     }
        //     printf("Yes: %s\n",buffer1);
        // }
    }

    // Close the socket
    close(sock);

    return 0;
}
