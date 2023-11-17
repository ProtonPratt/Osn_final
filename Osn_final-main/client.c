#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 5566
#define MAX_BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

int main() 
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

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
        printf("Enter a message to send to the server (type 'exit' to quit): ");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);

        // Send data to the server
        if (send(sock, buffer, strlen(buffer), 0) == -1) {
            perror("Sending data to server failed");
            break;
        }

        if(strncmp(buffer,"READ",4)==0 || strncmp(buffer,"WRITE",5)==0 || strncmp(buffer,"GETINFO",7)==0)
        {
            char buffer1[MAX_BUFFER_SIZE]="dsf";
            if (recv(sock, buffer1, sizeof(buffer1), 0) == -1) 
            {
                perror("Receiving data from server failed");
                close(sock);
                exit(EXIT_FAILURE);
            }

            printf("Yes: %s\n",buffer1);
        }

        else if(strncmp(buffer,"CREATE",6)==0 || strncmp(buffer,"DELETE",6)==0)
        {
            char buffer1[MAX_BUFFER_SIZE];
            if (recv(sock, buffer1, sizeof(buffer1), 0) == -1) 
            {
                perror("Receiving data from server failed");
                close(sock);
                exit(EXIT_FAILURE);
            }
            printf("Yes: %s\n",buffer1);
        }

        else if(strncmp(buffer,"COPY",4)==0)
        {
            char buffer1[MAX_BUFFER_SIZE];
            if (recv(sock, buffer1, sizeof(buffer1), 0) == -1) 
            {
                perror("Receiving data from server failed");
                close(sock);
                exit(EXIT_FAILURE);
            }
            printf("Yes: %s\n",buffer1);
        }
    }

    // Close the socket
    close(sock);

    return 0;
}