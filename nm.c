#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

// #define SERVER_PORT 8080
// #define CLIENT_PORT 5566
int server_port=8080;
int client_port=5566;
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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_connections(void *arg);
void *handle_client(void *arg);
void *handle_connections_client(void *arg);

int main() {
    pthread_t server_thread, client_thread;

    // Create a thread for handling server connections
    if (pthread_create(&server_thread, NULL, handle_connections, (void *)&server_port) < 0) {
        perror("Server thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Create a thread for handling client connections
    if (pthread_create(&client_thread, NULL, handle_connections_client, (void *)&client_port) < 0) {
        perror("Client thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish (this won't happen as they run in an infinite loop)
    pthread_join(server_thread, NULL);
    pthread_join(client_thread, NULL);

    return 0;
}

void *handle_connections(void *arg) {
    int port = *((int *)arg);

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
    address.sin_port = htons(port);

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

    printf("Server listening on port %d...\n", port);

    while (1) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Find an available slot in the array
        pthread_mutex_lock(&mutex);

        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].client_socket == 0) {
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

        pthread_mutex_unlock(&mutex);
    }
}

void *handle_client(void *arg) {
    struct ClientInfo *client_info = (struct ClientInfo *)arg;
    int client_socket = client_info->client_socket;
    char buffer[1024];
    int valread;

    // Read the data sent by the client
    valread = read(client_socket, buffer, sizeof(buffer));
    if (valread <= 0) {
        // Client disconnected or an error occurred
        close(client_socket);
    } else {
        char *token;
        int count = 0;

        token = strtok(buffer, " ");

        while (token != NULL) {
            if (count == 0) {
                strcpy(client_info->client_ip, token);
            } else if (count == 1) {
                client_info->server_port = atoi(token);
            } else if (count == 2) {
                client_info->client_port = atoi(token);
                client_info->id = client_info->client_port - client_info->server_port;
            }
            token = strtok(NULL, " ");
            count++;
        }

        printf("%s %d %d %d %d\n", client_info->client_ip, client_info->client_socket, client_info->client_port, client_info->server_port, client_info->id);

        // Send an acknowledgment back to the client
        char ack[] = "Acknowledgment: Data received";
        write(client_socket, ack, strlen(ack));
    }

    // Clear the client socket in the array
    // pthread_mutex_lock(&mutex);
    // client_info->client_socket = 0;
    // pthread_mutex_unlock(&mutex);

    // // Close the client socket
    // close(client_socket);

    // Exit the thread
    pthread_exit(NULL);
}

void *handle_connections_client(void *arg)
{
    int port = *((int *)arg);

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
    address.sin_port = htons(port);

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

    printf("Server listening on port %d for client...\n", port);

    while (1) 
    {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Find an available slot in the array
        pthread_mutex_lock(&mutex);

        // struct ClientInfo *client_info = (struct ClientInfo *)arg;
        // int client_socket = client_info->client_socket;
        char buffer[1024];
        int valread;

        // printf("1\n");
        // Read the data sent by the client
        valread = read(new_socket, buffer, sizeof(buffer));
        
        if (valread <= 0) 
        {
            // printf("done 1\n");
            // Client disconnected or an error occurred
            close(new_socket);
        } 
        
        else 
        {
            printf("Command:%s\n",buffer);
            // Send an acknowledgment back to the client
            char ack[] = "Acknowledgment: Data received";
            printf("Ack: %s\n",ack);
            write(new_socket, ack, strlen(ack));
        }

        pthread_mutex_unlock(&mutex);
    }
}
