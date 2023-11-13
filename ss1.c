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

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define CLIENT_PORT 8081
#define MAX_PATH_LENGTH 1024
#define MAX_PATHS 128



void listFilesRecursive(const char *dirPath, const char *basePath, char (*paths)[MAX_PATH_LENGTH], int *count) {
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(dirPath);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // Loop through each entry in the directory
    while ((entry = readdir(dir)) != NULL && *count < MAX_PATHS) {
        // Ignore "." and ".." entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Construct full path of the current entry
            char path[MAX_PATH_LENGTH];
            snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);

            // Check if the entry is a directory
            if (entry->d_type == DT_DIR) {
                // Recursively list files in the subdirectory
                listFilesRecursive(path, basePath, paths, count);
            } else {
                // Store the relative path of the file with a forward slash
                // prefixed at the beginning of the relative path
                snprintf(paths[(*count)++], MAX_PATH_LENGTH, "/%s", path + strlen(basePath) + 1);
            }
        }
    }

    // Close the directory
    closedir(dir);
}







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

    

     char currentDir[1024];
    if (getcwd(currentDir, sizeof(currentDir)) == NULL) {
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
snprintf(data, sizeof(data), "%s %d %d %d ", SERVER_IP, SERVER_PORT, CLIENT_PORT, num_paths);

// Append the paths to the data string
for (int i = 0; i < num_paths; ++i) {
    strncat(data, paths[i], sizeof(data) - strlen(data) - 1);
    strncat(data, " ", sizeof(data) - strlen(data) - 1);
}

// Send the data to the server
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
