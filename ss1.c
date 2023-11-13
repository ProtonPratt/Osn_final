#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define CLIENT_PORT 8081

int main() {
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

    // Send the IP address, SERVER_PORT, and CLIENT_PORT to the server
    char data[100];
    snprintf(data, sizeof(data), "%s %d %d", SERVER_IP, SERVER_PORT, CLIENT_PORT);
    send(sock, data, strlen(data), 0);
    printf("Data sent to the server: %s\n", data);

    // Receive the server's response
    valread = read(sock, buffer, sizeof(buffer));
    printf("Server response: %s\n", buffer);

    char path[100];

    strcpy(path,"/home/lenovo/Documents/OSN/Course_Project/test2/gay.txt");

    // Create new file

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

    // Create directory

    // if (mkdir(path, 0777) == 0) {
    //     printf("Directory created successfully.\n");
    // } else {
    //     printf("Error creating the directory.\n");
    // }

    // Delete the directory

    // if (rmdir(path) == 0) {
    //     printf("Directory deleted successfully.\n");
    // } else {
    //     printf("Error deleting the directory.\n");
    // }

    // Delete the file

    // if (remove(path) == 0) {
    //     printf("File deleted successfully.\n");
    // } else {
    //     printf("Error deleting the file.\n");
    // }

    return 0;
}
