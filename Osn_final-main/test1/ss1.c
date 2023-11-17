#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define NM_PORT 5567
#define CLIENT_PORT 8081
#define MAX_PATH_LENGTH 1024
#define MAX_PATHS 128

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int nm_port = NM_PORT;
int client_port = 8081;

void *handle_clients(void *arg);
void *handle_nm(void *arg);
void handle_read(int client_socket, const char *path);
void listFilesRecursive(const char *dirPath, const char *basePath, char (*paths)[MAX_PATH_LENGTH], int *count);
void *handle_client(void *arg);

void handle_read(int client_socket, const char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        // Handle the error (send an appropriate message to the client or take other actions)
        return;
    }

    // Read the content of the file and send it to the client
    char buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        write(client_socket, buffer, bytesRead);
    }

    fclose(file);
}


void listFilesRecursive(const char *dirPath, const char *basePath, char (*paths)[MAX_PATH_LENGTH], int *count)
{
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(dirPath);
    if (!dir)
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // Loop through each entry in the directory
    while ((entry = readdir(dir)) != NULL && *count < MAX_PATHS)
    {
        // Ignore "." and ".." entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            // Construct full path of the current entry
            char path[MAX_PATH_LENGTH];
            snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);

            printf("%s\n",path);

            // Store the relative path of the file with a forward slash
            // prefixed at the beginning of the relative path
            snprintf(paths[(*count)++], MAX_PATH_LENGTH, "/%s", path + strlen(basePath) + 1);

            // Check if the entry is a directory
            if (entry->d_type == DT_DIR)
            {
                // Recursively list files in the subdirectory
                listFilesRecursive(path, basePath, paths, count);
            }
        }
    }

    // Close the directory
    closedir(dir);
}


void *handle_client(void *arg)
{
    int client_port = *((int *)arg);
    int client_socket;

    // Create a socket for the client
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client_address;
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;
    client_address.sin_port = htons(client_port);

    // Connect to the client using the specified port
    if (connect(client_socket, (struct sockaddr *)&client_address, sizeof(client_address)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    int valread;

    // Read the data sent by the server
    valread = read(client_socket, buffer, sizeof(buffer));
    if (valread <= 0)
    {
        // Server disconnected or an error occurred
        close(client_socket);
    }
    else
    {
        printf("Server Command: %s\n", buffer);

        // Construct client information
        char client_info[1024];
        snprintf(client_info, sizeof(client_info), "Client IP: %s\nClient Port: %d", SERVER_IP, client_port);
        write(client_socket, client_info, strlen(client_info));

        char ack[] = "Acknowledgment: Data received";

        // Process the server command
        if (strncmp(buffer, "READ", 4) == 0)
        {
            char *pathStart = strchr(buffer, ' ') + 1;
            size_t pathLength = strlen(pathStart) - 1;
            char *path = (char *)malloc(pathLength + 1);
            strncpy(path, pathStart, pathLength);
            path[pathLength] = '\0';

            // Call the function to handle the read operation and send content to the client
            handle_read(client_socket, path);
        }
        // Add additional else if conditions for other commands like WRITE, DELETE, etc.

        // Send an acknowledgment back to the server
        write(client_socket, ack, strlen(ack));
    }

    // Close the client socket
    close(client_socket);

    // Exit the thread
    pthread_exit(NULL);
}

void *handle_nm(void *arg)
{
    int nm_fd, client_socket;
    struct sockaddr_in nm_address, client_address;
    socklen_t client_addrlen = sizeof(client_address);

    // Create socket for the Naming Server
    if ((nm_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    nm_address.sin_family = AF_INET;
    nm_address.sin_addr.s_addr = INADDR_ANY;
    nm_address.sin_port = htons(nm_port);

    // Bind the socket
    if (bind(nm_fd, (struct sockaddr *)&nm_address, sizeof(nm_address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(nm_fd, 5) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Naming Server listening on port %d...\n", nm_port);

    while (1)
    {
        // Accept a new connection from a Storage Server
        if ((client_socket = accept(nm_fd, (struct sockaddr *)&client_address, &client_addrlen)) < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Handle the new connection in a separate thread
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)&client_socket) < 0)
        {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
}

int main()
{

    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(nm_port);
    // addr.sin_addr.s_addr = inet_addr(ip);

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    char currentDir[1024];
    if (getcwd(currentDir, sizeof(currentDir)) == NULL)
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    // Create an array to store paths
    char paths[MAX_PATHS][MAX_PATH_LENGTH];
    int num_paths = 0;

    // List files in the current directory
    listFilesRecursive(currentDir, currentDir, paths, &num_paths);

    // // Send the IP address, SERVER_PORT, and CLIENT_PORT to the server and also send the list of accessible paths
    // char data[100];
    // snprintf(data, sizeof(data), "%s %d %d", SERVER_IP, SERVER_PORT, CLIENT_PORT);
    // send(sock, data, strlen(data), 0);
    // printf("Data sent to the server: %s\n", data);

    // Create a buffer to hold the data to be sent
    char data[4096]; // Adjust the size as needed

    // Format the data string with SERVER_IP, SERVER_PORT, CLIENT_PORT, and paths
    char resolved_path[MAX_PATH_LENGTH];
    realpath("ss1.c", resolved_path);

    snprintf(data, sizeof(data), "%s %d %d %d %s", SERVER_IP, nm_port, CLIENT_PORT, num_paths, resolved_path);

    // Append the paths to the data string
    for (int i = 0; i < num_paths; ++i)
    {
        strncat(data, paths[i], sizeof(data) - strlen(data) - 1);
        strncat(data, " ", sizeof(data) - strlen(data) - 1);
    }

    // Send the data to the server
    send(sock, data, strlen(data), 0);
    printf("Data sent to the server: %s\n", data);

    // Receive the server's response
    valread = read(sock, buffer, sizeof(buffer));
    printf("Server response: %s\n", buffer);

    pthread_t nm_thread, client_thread;

    // Create a thread for handling server connections
    if (pthread_create(&nm_thread, NULL, handle_nm, (void *)&nm_port) < 0)
    {
        perror("Server thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Create a thread for handling client connections
    if (pthread_create(&client_thread, NULL, handle_client, (void *)&client_port) < 0)
    {
        perror("Client thread creation failed");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish (this won't happen as they run in an infinite loop)
    pthread_join(nm_thread, NULL);
    pthread_join(client_thread, NULL);

    // char path[100];

    // strcpy(path,"/home/lenovo/Documents/OSN/Course_Project/test2/gay.txt");

    // // Create new file

    // struct stat info;
    // if (stat(path, &info) != 0) {
    //     // If stat fails, assume it's a new file
    //     FILE *file = fopen(path, "w");
    //     if (file != NULL) {
    //         printf("File created successfully.\n");
    //         fclose(file);
    //     } else {
    //         printf("Error creating the file.\n");
    //     }
    // }

    // // Create directory

    // if (mkdir(path, 0777) == 0) {
    //     printf("Directory created successfully.\n");
    // } else {
    //     printf("Error creating the directory.\n");
    // }

    // // Delete the directory

    // if (rmdir(path) == 0) {
    //     printf("Directory deleted successfully.\n");
    // } else {
    //     printf("Error deleting the directory.\n");
    // }

    // // Delete the file

    // if (remove(path) == 0) {
    //     printf("File deleted successfully.\n");
    // } else {
    //     printf("Error deleting the file.\n");
    // }

    return 0;
}
