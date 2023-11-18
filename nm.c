#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "search.h"

// #define SERVER_PORT 8080
// #define CLIENT_PORT 5566
int ss_to_nm_port=8080;
int client_to_nm_port=5566;
#define MAX_SS 5
#define MAX_SS_PATHS 128
#define MAX_SS_PATH_LENGTH 1024
int num_ss=0;
int num_clients=0;
#define MAX_CLIENTS 5

// Define a structure to store client data
struct SS_Info {
    int ss_socket;
    char ss_ip[16];  // Store IP address as a string
    int server_port;
    int ss_port;
    int id;
     int num_paths;
     char paths[MAX_SS_PATHS][MAX_SS_PATH_LENGTH];
};

struct Client_Info{
     int client_socket;
     
};

// struct Client_Info client_arr[MAX_CLIENTS];
// struct SS_Info ss_arr[MAX_SS];


struct SS_Info *ss_arr = NULL;
struct Client_Info *client_arr = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_connections_ss(void *arg);
void *handle_ss(void *arg);
void *handle_client(void *arg);
void *handle_connections_client(void*arg);
void addClient(int client_socket);
void addSS(int ss_socket);



void removeLastSlash(char *path) {
    // Find the last occurrence of '/'
    char *lastSlash = strrchr(path, '/');

    if (lastSlash != NULL) {
        // Null-terminate the string at the last slash
        *lastSlash = '\0';
    }
}


void addSS(int ss_socket) {
    // Allocate memory for a new SS_Info struct
    ss_arr = realloc(ss_arr, (num_ss + 1) * sizeof(struct SS_Info));

    // Check if memory allocation was successful
    if (ss_arr == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize the new SS_Info struct
    struct SS_Info *new_ss = &ss_arr[num_ss];
    new_ss->ss_socket = ss_socket;
   
    // Increment the number of SS in the system
    num_ss++;
}

// Function to add a new client to the system
void addClient(int client_socket) {
    // Allocate memory for a new Client_Info struct
    client_arr = realloc(client_arr, (num_clients + 1) * sizeof(struct Client_Info));

    // Check if memory allocation was successful
    if (client_arr == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize the new Client_Info struct
    struct Client_Info *new_client = &client_arr[num_clients];
    new_client->client_socket = client_socket;

    // Increment the number of clients in the system
    num_clients++;
}

// void search(){
    
// }

int main() {
    pthread_t server_thread, client_thread;

    // Create a thread for handling server connections
    if (pthread_create(&server_thread, NULL, handle_connections_ss, (void *)&ss_to_nm_port) < 0) {
        perror("Server thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Create a thread for handling client connections
    if (pthread_create(&client_thread, NULL, handle_connections_client, (void *)&client_to_nm_port) < 0) {
        perror("Client thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish (this won't happen as they run in an infinite loop)
    pthread_join(server_thread, NULL);
    pthread_join(client_thread, NULL);

    return 0;
}

void *handle_connections_ss(void *arg) {
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
    if (listen(server_fd, MAX_SS) < 0) {
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

        // int i;
        // for (i = 0; i < MAX_SS; i++) {
        //     if (ss_arr[i].ss_socket == 0) 
        //     {
        //         num_ss=i+1;
        //         ss_arr[i].ss_socket = new_socket;

        //         // Create a thread to handle the new connection
        //         pthread_t thread;
        //         if (pthread_create(&thread, NULL, handle_ss, (void *)&ss_arr[i]) < 0) {
        //             perror("Thread creation failed");
        //             exit(EXIT_FAILURE);
        //         }
        //         break;
        //     }
        // }

        addSS(new_socket);
         pthread_t thread;
                if (pthread_create(&thread, NULL, handle_ss, (void *)&ss_arr[num_ss-1]) < 0) {
                    perror("Thread creation failed");
                    exit(EXIT_FAILURE);
                }


        pthread_mutex_unlock(&mutex);
    }
}

void *handle_ss(void *arg) 
{
    struct SS_Info *ss_info = (struct SS_Info *)arg;
    int ss_socket = ss_info->ss_socket;
    char buffer[4096]; // Adjust the size as needed
    int valread;

    // Read the data sent by the client
    // valread = read(client_socket, buffer, sizeof(buffer));
    // if (valread <= 0) {
    //     // Client disconnected or an error occurred
    //     close(client_socket);
    // } else {
        recv(ss_socket, buffer, sizeof(buffer),0);
        char *token;
        int count = 0;

        token = strtok(buffer, " ");

        while (token != NULL) {
            if (count == 0) {
                strcpy(ss_info->ss_ip, token);
            } else if (count == 1) {
                ss_info->server_port = atoi(token);
            } else if (count == 2) {
                ss_info->ss_port = atoi(token);
                ss_info->id = ss_info->ss_port - ss_info->server_port;
            } else if (count == 3) {
                ss_info->num_paths = atoi(token);
            } else if (count >= 4 && count < 4 + ss_info->num_paths) {
                // Copy paths into the ClientInfo structure
                strcpy(ss_info->paths[count - 4], token);
            }
            token = strtok(NULL, " ");
            count++;
        }

        printf("Client Info:\n");
        printf("Socket: %d\n",ss_info->ss_socket);
        printf("IP: %s\n", ss_info->ss_ip);
        printf("Server Port: %d\n", ss_info->server_port);
        printf("Client Port: %d\n", ss_info->ss_port);
        printf("ID: %d\n", ss_info->id);
        printf("Number of Paths: %d\n", ss_info->num_paths);
        printf("Paths:\n");
        for (int i = 0; i < ss_info->num_paths; ++i) {
            printf("%s\n", ss_info->paths[i]);
        }

        // Send an acknowledgment back to the client
        char ack[1024];
        strcpy(ack,"Acknowledgment: Data received");
        printf("Client: %d\n",ss_socket);
        send(ss_socket, ack, sizeof(ack),0);

    // }

    // Close the client socket
    // close(client_socket);

    // Exit the thread
    pthread_exit(NULL);
}


void*handle_connections_client(void*arg)
{
    int port=*((int *)arg);


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

     while (1) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Find an available slot in the array
        pthread_mutex_lock(&mutex);

        // int i;
        // for (i = 0; i < MAX_CLIENTS; i++) {
        //     if (client_arr[i].client_socket == 0) 
        //     {
        //         num_client=i+1;
        //         client_arr[i].client_socket = new_socket;

        //         // Create a thread to handle the new connection
        //         pthread_t thread;
        //         if (pthread_create(&thread, NULL, handle_client, (void *)&client_arr[i]) < 0) {
        //             perror("Thread creation failed");
        //             exit(EXIT_FAILURE);
        //         }
        //         break;
        //     }
        // }
        addClient(new_socket);
        pthread_t thread;
                   if (pthread_create(&thread, NULL, handle_client, (void *)&client_arr[num_clients-1]) < 0) {
                    perror("Thread creation failed");
                    exit(EXIT_FAILURE);
                }
        pthread_mutex_unlock(&mutex);
    }


    



}






void *handle_client(void *arg)
{
    // int port = *((int *)arg);

    // int server_fd, new_socket;
    // struct sockaddr_in address;
    // int addrlen = sizeof(address);

    // // Create socket
    // if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    //     perror("Socket creation failed");
    //     exit(EXIT_FAILURE);
    // }

    // address.sin_family = AF_INET;
    // address.sin_addr.s_addr = INADDR_ANY;
    // address.sin_port = htons(port);

    // // Bind the socket
    // if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    //     perror("Bind failed");
    //     exit(EXIT_FAILURE);
    // }

    // // Listen for incoming connections
    // if (listen(server_fd, MAX_SS) < 0) {
    //     perror("Listen failed");
    //     exit(EXIT_FAILURE);
    // }

    // printf("Server listening on port %d for client...\n", port);

    // if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) 
    // {
    //         perror("Accept failed");
    //         exit(EXIT_FAILURE);
    // }

  struct Client_Info *client_info = (struct Client_Info *)arg;
    int clienta_socket = client_info->client_socket;
    



    char buffer[1024];
    char data[4096];
    while (1) 
    {
        bzero(buffer,sizeof(buffer));

        recv(clienta_socket, buffer, sizeof(buffer),0);

        char ack[]="This is an ACK for the given command issued by the client";
        // sleep(6);
        send(clienta_socket,ack,sizeof(ack),0);

        pthread_mutex_lock(&mutex);
            
            if(strncmp(buffer,"READ",4)==0 || strncmp(buffer,"WRITE",5)==0 || strncmp(buffer,"GETINFO",7)==0)
            {
                char *token;
                int count = 0;

                token = strtok(buffer, " ");
                char temp[MAX_SS_PATH_LENGTH]; // temp relative path
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

                // search
                for(int i=0;i<num_ss;i++)
                {
                    for(int j=0;j<ss_arr[i].num_paths;j++)
                    {
                        if(strcmp(ss_arr[i].paths[j],temp)==0)
                        {
                            strcpy(ip_temp,ss_arr[i].ss_ip);
                            temp_port=ss_arr[i].ss_port;
                            break;
                        }
                    }
                }

                bzero(data,sizeof(data));
                printf("Port: %d\n",temp_port);
                snprintf(data, sizeof(data), "%s %d", ip_temp, temp_port);
                printf("Sent Data: %s\n",data);
                send(clienta_socket, data, sizeof(data), 0);
            }
            else if(strncmp(buffer,"CREATE",6)==0 || strncmp(buffer,"DELETE",6)==0)
            {
                char *token;
                int count = 0;
                char buffer1[2048];
                strcpy(buffer1,buffer);

                token = strtok(buffer, " ");
                char temp[MAX_SS_PATH_LENGTH];
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
                    // search
                    // for(int i=0;i<num_ss;i++)
                    // {
                    //     for(int j=0;j<ss_arr[i].num_paths;j++)
                    //     {
                    //         if(strcmp(ss_arr[i].paths[j],temp)==0)
                    //         {   
                    //             // printf("Temp Socket: %d\n",temp_socket);
                    //             temp_socket=ss_arr[i].ss_socket;
                    //             printf("Temp Socket: %d\n",temp_socket);
                    //             break;
                    //         }
                    //     }
                    // }



                          for (int i = 0; i < num_ss; i++) {
                              for (int j = 0; j < ss_arr[i].num_paths; j++) {
                                     if (strcmp(ss_arr[i].paths[j], temp) == 0) {
                                       // Path found, remove it from ss_arr
                                      temp_socket = ss_arr[i].ss_socket;
                                      printf("Temp Socket: %d\n", temp_socket);

                                        // Shift the remaining elements in the array to fill the gap
                        for (int k = j; k < ss_arr[i].num_paths - 1; k++) {
                         strcpy(ss_arr[i].paths[k], ss_arr[i].paths[k + 1]);
                          }

            // Decrease the number of paths in the array
                          ss_arr[i].num_paths--;

                             break;
        }
    }
}

                    // printf("Buffer: %s\n",buffer);
                    send(temp_socket,buffer1,sizeof(buffer1),0);
                    printf("Send\n");
                    bzero(buffer1,sizeof(buffer1));
                    recv(temp_socket,buffer1,sizeof(buffer1),0);
                    send(clienta_socket, buffer1, sizeof(buffer1),0);

                }

                else if(strncmp(buffer,"CREATE",6)==0)
                {
                    char temp1[MAX_SS_PATH_LENGTH];
                    strcpy(temp1,temp);
                    
                    // search
                    removeLastSlash(temp);
                    for(int i=0;i<num_ss;i++)
                    {
                        for(int j=0;j<ss_arr[i].num_paths;j++)
                        {
                            // printf("Comp1: %s Comp2: %s\n",temp_path1,temp);

                            if(strcmp(ss_arr[i].paths[j],temp)==0)
                            {
                                temp_socket=ss_arr[i].ss_socket;
                                printf("Temp Socket: %d\n",temp_socket);
                                strcpy(ss_arr[i].paths[ss_arr[i].num_paths],temp1);
                                ss_arr[i].num_paths++;
                                break;
                            }
                        }
                    }
                    send(temp_socket,buffer1,sizeof(buffer1),0);
                    printf("Send\n");
                    bzero(buffer1,sizeof(buffer1));
                    recv(temp_socket,buffer1,sizeof(buffer1),0);
                    send(clienta_socket, buffer1, sizeof(buffer1),0);
                }
            }

        pthread_mutex_unlock(&mutex);
    }
    close(clienta_socket);
}
