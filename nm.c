#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define CLIENT_PORT 5566
#define MAX_CLIENTS 5

// Define a structure to store client data
struct ClientInfo {
    int client_socket;
    char client_ip[16];  // Store IP address as a string
    int server_port;
    int client_port;
    int id;
};

struct ClientInfo clients[MAX_CLIENTS];

void *handle_client(void *arg);

int main() 
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Create an array to store client data
    // struct ClientInfo clients[MAX_CLIENTS];

    while (1) 
    {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }


        // Find an available slot in the array
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) 
        {
            if (clients[i].client_socket == 0) 
            {
                clients[i].client_socket = new_socket;
                // Create a thread to handle the new connection
                pthread_t thread;
                if (pthread_create(&thread, NULL, handle_client, (void *)&clients[i]) < 0) {
                    perror("Thread creation failed");
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }
    }

    return 0;
}

void *handle_client(void *arg) 
{
    struct ClientInfo *client_info = (struct ClientInfo *)arg;
    int client_socket = client_info->client_socket;
    char buffer[1024];
    int valread;

    // Read the data sent by the client
    valread = read(client_socket, buffer, sizeof(buffer));
    if (valread <= 0) 
    {
        // Client disconnected or an error occurred
        close(client_socket);
    } 
    else 
    {

        char *token;
        int count=0;

        token = strtok(buffer, " ");
    
        while (token != NULL) 
        {
            if(count==0)
            {
                strcpy(client_info->client_ip,token);
            }
            else if(count==1)
            {
                client_info->server_port=atoi(token);
            }
            else if(count==2)
            {
                client_info->client_port=atoi(token);
                client_info->id=client_info->client_port-client_info->server_port;
            }
            // printf("Token: %s\n", token);
            token = strtok(NULL, " ");
            count++;
        }

        printf("%s %d %d %d %d\n",client_info->client_ip,client_info->client_socket,client_info->client_port,client_info->server_port,client_info->id);

        // Send an acknowledgment back to the client
        char ack[] = "Acknowledgment: Data received";
        write(client_socket, ack, strlen(ack));
    }

    // Clear the client socket in the array
    // client_info->client_socket = 0;

    // Close the client socket

    // close(client_socket);

    // Exit the thread
    pthread_exit(NULL);
}
