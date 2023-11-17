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
#define MAX_CLIENT_PATHS 128
#define MAX_CLIENT_PATH_LENGTH 1024
int num_ss=0;

// Define a structure to store client data
struct ClientInfo {
    int client_socket;
    char client_ip[16];  // Store IP address as a string
    int server_port;
    int client_port;
    int id;
     int num_paths;
     char paths[MAX_CLIENT_PATHS][MAX_CLIENT_PATH_LENGTH];
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
            if (clients[i].client_socket == 0) 
            {
                num_ss=i+1;
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

void *handle_client(void *arg) 
{
    struct ClientInfo *client_info = (struct ClientInfo *)arg;
    int client_socket = client_info->client_socket;
    char buffer[4096]; // Adjust the size as needed
    int valread;

    // Read the data sent by the client
    // valread = read(client_socket, buffer, sizeof(buffer));
    // if (valread <= 0) {
    //     // Client disconnected or an error occurred
    //     close(client_socket);
    // } else {
        recv(client_socket, buffer, sizeof(buffer),0);
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
            } else if (count == 3) {
                client_info->num_paths = atoi(token);
            } else if (count >= 4 && count < 4 + client_info->num_paths) {
                // Copy paths into the ClientInfo structure
                strcpy(client_info->paths[count - 4], token);
            }
            token = strtok(NULL, " ");
            count++;
        }

        printf("Client Info:\n");
        printf("Socket: %d\n",client_info->client_socket);
        printf("IP: %s\n", client_info->client_ip);
        printf("Server Port: %d\n", client_info->server_port);
        printf("Client Port: %d\n", client_info->client_port);
        printf("ID: %d\n", client_info->id);
        printf("Number of Paths: %d\n", client_info->num_paths);
        printf("Paths:\n");
        for (int i = 0; i < client_info->num_paths; ++i) {
            printf("%s\n", client_info->paths[i]);
        }

        // Send an acknowledgment back to the client
        char ack[1024];
        strcpy(ack,"Acknowledgment: Data received");
        printf("Client: %d\n",client_socket);
        send(client_socket, ack, sizeof(ack),0);

    // }

    // Close the client socket
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

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) 
    {
            perror("Accept failed");
            exit(EXIT_FAILURE);
    }

    char buffer[1024];
    while (1) 
    {
        // Accept a new connection

        // Find an available slot in the array

        // struct ClientInfo *client_info = (struct ClientInfo *)arg;
        // int client_socket = client_info->client_socket;
        // char buffer[1024];
        // int valread;

        // printf("1\n");
        // Read the data sent by the client
        // valread = read(new_socket, buffer, sizeof(buffer));
        
        // if (valread <= 0) 
        // {
        //     printf("done 1\n");
        //     // Client disconnected or an error occurred
        // } 
        bzero(buffer,sizeof(buffer));

        recv(new_socket, buffer, sizeof(buffer),0);

        pthread_mutex_lock(&mutex);

        // else 
        // {
            printf("Command:%s\n",buffer);
            // Send an acknowledgment back to the client
            char ack[] = "Acknowledgment: Data received";
            
            if(strncmp(buffer,"READ",4)==0 || strncmp(buffer,"WRITE",5)==0 || strncmp(buffer,"GETINFO",7)==0)
            {
                char *token;
                int count = 0;

                token = strtok(buffer, " ");
                char temp[MAX_CLIENT_PATH_LENGTH];
                char ip_temp[1024];
                int temp_port;

                while(token!=NULL)
                {
                    if(count==1)
                    {
                        strcpy(temp,token);
                    }

                    token = strtok(NULL, " ");
                    count++;
                }

                // printf("%d\n",num_ss);
                for(int i=0;i<num_ss;i++)
                {
                    for(int j=0;j<clients[i].num_paths;j++)
                    {
                        if(strcmp(clients[i].paths[j],temp)==0)
                        {
                            strcpy(ip_temp,clients[i].client_ip);
                            temp_port=clients[i].client_port;
                            break;
                        }
                    }
                }

                char data[4096];
                printf("Port: %d\n",temp_port);
                snprintf(data, sizeof(data), "%s %d", ip_temp, temp_port);
                send(new_socket, data, sizeof(data), 0);
            }
            else if(strncmp(buffer,"CREATE",6)==0 || strncmp(buffer,"DELETE",6)==0)
            {
                char *token;
                int count = 0;
                char buffer1[2048];
                strcpy(buffer1,buffer);

                token = strtok(buffer, " ");
                char temp[MAX_CLIENT_PATH_LENGTH];
                int temp_socket;

                while(token!=NULL)
                {
                    if(count==1)
                    {
                        strcpy(temp,token);
                    }

                    token = strtok(NULL, " ");
                    count++;
                }

                if(strncmp(buffer,"DELETE",6)==0)
                {
                    for(int i=0;i<num_ss;i++)
                    {
                        for(int j=0;j<clients[i].num_paths;j++)
                        {
                            if(strcmp(clients[i].paths[j],temp)==0)
                            {
                                // printf("Temp Socket: %d\n",temp_socket);
                                temp_socket=clients[i].client_socket;
                                printf("Temp Socket: %d\n",temp_socket);
                                break;
                            }
                        }
                    }
                    // printf("Buffer: %s\n",buffer);
                    send(temp_socket,buffer1,sizeof(buffer1),0);
                    printf("Send\n");
                    bzero(buffer1,sizeof(buffer1));
                    recv(temp_socket,buffer1,sizeof(buffer1),0);
                    send(new_socket, buffer1, sizeof(buffer1),0);

                    // struct sockaddr_in serv_addr;
                    
                    // // Create socket
                    // if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    //     perror("Socket creation failed");
                    //     exit(EXIT_FAILURE);
                    // }

                    // serv_addr.sin_family = AF_INET;
                    // serv_addr.sin_port = htons(SERVER_PORT);

                    // // Convert IP address from string to binary form
                    // if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
                    //     perror("Invalid address/ Address not supported");
                    //     exit(EXIT_FAILURE);
                    // }

                    // // Connect to the server
                    // if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                    //     perror("Connection failed");
                    //     exit(EXIT_FAILURE);
                    // }

                }
            }
        // }

        pthread_mutex_unlock(&mutex);
    }
    close(new_socket);
}
