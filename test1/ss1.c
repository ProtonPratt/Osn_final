#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define CLIENT_PORT 8081
#define MAX_PATH_LENGTH 1024
#define MAX_PATHS 128
int server_port=SERVER_PORT;
int client_port=CLIENT_PORT;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_CLIENT_PATH_LENGTH 4096
#define TIMEOUT_SECONDS 2
int sock=0;

int containsChar(const char *str, char target) {
    while (*str != '\0') {
        if (*str == target) {
            return 1; // Character found
        }
        str++;
    }
    return 0; // Character not found
}

ssize_t recv_with_timeout(int sockfd, void *buf, size_t len, int flags, int timeout_seconds) {
    fd_set readfds;
    struct timeval timeout;
    ssize_t bytesRead;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;

    int ready = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

    if (ready == -1) {
        perror("Error in select");
        exit(1);
    } else if (ready == 0) {
        // Timeout occurred
        return 0;
    } else {
        // Socket is ready for reading
        bytesRead = recv(sockfd, buf, len, flags);
        return bytesRead;
    }
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

            // printf("%s\n",path);

            // Store the relative path of the file with a forward slash
            // prefixed at the beginning of the relative path
            snprintf(paths[(*count)++], MAX_PATH_LENGTH, "%s", path + strlen(basePath) + 1);

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

void removeLastCharacter(char *str) {
    size_t len = strlen(str);

    // Check if the string is not empty
    if (len > 0) {
        // Set the last character to null terminator
        str[len - 1] = '\0';
    }
}

void send_file(const char *filename, int client_socket) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file for reading");
        exit(1);
    }

    char buffer[MAX_CLIENT_PATH_LENGTH];
    size_t bytesRead;
    bzero(buffer,sizeof(buffer));
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) 
    {
        printf("Sendway: %s\n",buffer);
        if (send(client_socket, buffer, bytesRead, 0) == -1) {
            perror("Error sending file");
            exit(1);
        }
        bzero(buffer,sizeof(buffer));
    }
    // bzero(buffer,sizeof(buffer));
    // bytesRead = fread(buffer, 1, sizeof(buffer), file);
    // send(client_socket, buffer, bytesRead, 0);

    fclose(file);
}

void receive_and_create_file(const char *filename, int server_socket) 
{
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error creating file");
        exit(1);
    }

    char buffer[MAX_CLIENT_PATH_LENGTH];
    ssize_t bytesRead;

    printf("Midway 1\n");
    bzero(buffer,sizeof(buffer));
    while ((bytesRead = recv_with_timeout(server_socket, buffer, sizeof(buffer), 0,TIMEOUT_SECONDS)) > 0) 
    {
        // bzero(buffer,sizeof(buffer));
        // bytesRead = recv(server_socket, buffer, sizeof(buffer), 0);
        printf("Midway 2: %s\n",buffer);
        fwrite(buffer, 1, bytesRead, file);
        bzero(buffer,sizeof(buffer));
    }

    fclose(file);
}

void removeDirectoryRecursively(const char *path) 
{
    DIR *dir = opendir(path);
    struct dirent *entry;

    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip the "." and ".." entries
        }

        char entryPath[PATH_MAX];
        snprintf(entryPath, sizeof(entryPath), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            // Recursive call for subdirectories
            removeDirectoryRecursively(entryPath);
        } else {
            // Remove regular files
            if (remove(entryPath) != 0) {
                perror("Error removing file");
            }
        }
    }

    closedir(dir);

    // Remove the empty directory itself after its contents are removed
    if (rmdir(path) != 0) {
        perror("Error removing directory");
    }
}

void *handle_connections_client(void*arg)
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
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    char buffer[2048];

     while (1) 
    {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Find an available slot in the array
        // pthread_mutex_lock(&mutex);
        bzero(buffer,sizeof(buffer));
            recv(new_socket, buffer, sizeof(buffer),0);
            printf("Command:%s\n",buffer);
            char ack[] = "Acknowledgment: Data received";

            if(strncmp(buffer,"READ",4)==0)
            {
                char* token;
                int count = 0;

                token = strtok(buffer, " ");

                char temp[MAX_CLIENT_PATH_LENGTH];

                while(token!=NULL)
                {
                    if(count==1)
                    {
                        strcpy(temp,token);
                    }

                    token = strtok(NULL, " ");
                    count++;
                }

                char temp1[MAX_CLIENT_PATH_LENGTH];
                getcwd(temp1, sizeof(temp1));
                strcat(temp1,"/");
                strcat(temp1,temp);

                FILE* file=fopen(temp1,"rb");
                if (file == NULL) 
                {
                    perror("Error opening file");
                }

                char bet_buf[1000];
                size_t bytesRead;

                while ((bytesRead = fread(bet_buf, 1, sizeof(bet_buf), file)) > 0) 
                {   
                    fwrite(bet_buf, 1, bytesRead, stdout);
                    send(new_socket,bet_buf,sizeof(bet_buf),0);
                    bzero(bet_buf,sizeof(bet_buf));
                    recv(new_socket, bet_buf, sizeof(bet_buf),0);

                    if(strcmp(bet_buf,"GOT")!=0)
                    {
                        break;
                    }
                }
                bzero(bet_buf,sizeof(bet_buf));
                strcpy(bet_buf,"STOP");
                send(new_socket,bet_buf,sizeof(bet_buf),0);
                fclose(file);

            }

            else if(strncmp(buffer,"WRITE",5)==0)
            {
                pthread_mutex_lock(&mutex);
                char* token;
                int count = 0;

                token = strtok(buffer, " ");

                char temp[MAX_CLIENT_PATH_LENGTH];
                char to_write[MAX_CLIENT_PATH_LENGTH];
                bzero(temp,sizeof(temp));
                bzero(to_write,sizeof(to_write));

                while(token!=NULL)
                {
                    if(count==1)
                    {
                        strcpy(temp,token);
                    }
                    if(count>=2)
                    {
                        strcat(to_write,token);
                        strcat(to_write," ");
                    }

                    token = strtok(NULL, " ");
                    count++;
                }
                removeLastCharacter(to_write);

                char temp1[MAX_CLIENT_PATH_LENGTH];
                getcwd(temp1, sizeof(temp1));
                strcat(temp1,"/");
                strcat(temp1,temp);

                FILE* file=fopen(temp1,"a");
                if (file == NULL) 
                {
                    perror("Error opening file");
                }


                if (fputs(to_write, file) == EOF) 
                {
                    perror("Error appending data to file");
                    fclose(file);
                }

                bzero(to_write,sizeof(to_write));
                strcpy(to_write,"STOP");
                send(new_socket,to_write,sizeof(to_write),0);
                fclose(file);
                
                pthread_mutex_unlock(&mutex);
            }

            else if(strncmp(buffer,"GETINFO",7)==0)
            {
                char* token;
                int count = 0;

                token = strtok(buffer, " ");

                char temp[MAX_CLIENT_PATH_LENGTH];
                char to_write[MAX_CLIENT_PATH_LENGTH];

                while(token!=NULL)
                {
                    if(count==1)
                    {
                        strcpy(temp,token);
                    }

                    token = strtok(NULL, " ");
                    count++;
                }

                char temp1[MAX_CLIENT_PATH_LENGTH];
                getcwd(temp1, sizeof(temp1));
                strcat(temp1,"/");
                strcat(temp1,temp);

                 struct stat file_info;

                // Get file information using stat
                if (stat(temp1, &file_info) != 0) {
                    perror("Error getting file information");
                }

                snprintf(to_write, sizeof(to_write), "Size: %lld Permissions: %o", (long long)file_info.st_size,(unsigned int)(file_info.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)));
                send(new_socket,to_write,sizeof(to_write),0);
                bzero(to_write,sizeof(to_write));
                recv(new_socket, to_write, sizeof(to_write),0);

                bzero(to_write,sizeof(to_write));
                strcpy(to_write,"STOP");
                send(new_socket,to_write,sizeof(to_write),0);
            }


        close(new_socket);

        // pthread_mutex_unlock(&mutex);
    }

}

void * handle_connections_server(void* arg)
{
    int port=*((int *)arg);
    struct sockaddr_in serv_addr;
    char buffer[4096];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

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
    send(sock, data, sizeof(data), 0);
    printf("Data sent to the server: %s\n", data);

    // Receive the server's response
    recv(sock, buffer, sizeof(buffer),0);
    printf("Server response: %s\n", buffer);
    printf("Socket: %d\n",sock);

    while(1)
    {
        recv(sock, buffer, sizeof(buffer),0);
        printf("Command: %s\n", buffer);

        char *token = strtok(buffer, " ");
        char temp[MAX_CLIENT_PATH_LENGTH];
        int count=0;
        char flag[3];

        if(strncmp(buffer,"DELETE",6)==0)
        {
            while(token!=NULL)
                    {
                        if(count==1)
                        {
                            strcpy(temp,token);
                        }
                        if(count==2)
                        {
                            strcpy(flag,token);
                        }

                        token = strtok(NULL, " ");
                        count++;
                    }
            
            char temp1[MAX_CLIENT_PATH_LENGTH];
            getcwd(temp1, sizeof(temp1));
            strcat(temp1,"/");
            strcat(temp1,temp);

            if(strcmp(flag,"-f")==0)
            {
                remove(temp1);
            }

            else if(strcmp(flag,"-d")==0)
            {
                removeDirectoryRecursively(temp1);
            }

           
           for (int i = 0; i < num_paths; i++) {
        if (strcmp(paths[i], temp1) == 0) {
        // Path found, remove it from paths
        

        // Shift the remaining elements in the array to fill the gap
        for (int j = i; j < num_paths - 1; j++) {
            strcpy(paths[j], paths[j + 1]);
        }

        // Decrease the number of paths in the array
        num_paths--;

        break;
      }
   }


            char ack[1024];
            strcpy(ack,"STOP");

            send(sock,ack,sizeof(ack),0);
        }

        else if(strncmp(buffer,"CREATE",6)==0)
        {
            while(token!=NULL)
                    {
                        if(count==1)
                        {
                            strcpy(temp,token);
                        }
                        if(count==2)
                        {
                            strcpy(flag,token);
                        }

                        token = strtok(NULL, " ");
                        count++;
                    }
            
            char temp1[MAX_CLIENT_PATH_LENGTH];
            getcwd(temp1, sizeof(temp1));
            strcat(temp1,"/");
            strcat(temp1,temp);

            if(strcmp(flag,"-d")==0)
            {
                mkdir(temp1,S_IRWXU);
            }
            
            else if(strcmp(flag,"-f")==0)
            {
                FILE* file=fopen(temp1,"w");
                fclose(file);
            }

            char ack[1024];
            strcpy(ack,"STOP");

            send(sock,ack,sizeof(ack),0);
        }
        else if(strncmp(buffer,"COPY",4)==0)
        {
             int length=0;
            while (buffer[length] != '\n') 
            {
                length++;
            }
            
            printf("Last flag: %c\n",buffer[length-1]);
            if(buffer[length-1]=='0')
            {
                printf("0 achieved\n");
                while(token!=NULL)
                    {
                        if(count==1)
                        {
                            strcpy(temp,token);
                        }
                        if(count==2)
                        {
                            strcpy(flag,token);
                        }

                        token = strtok(NULL, " ");
                        count++;
                    }

                printf("Sent file: %s\n",temp);
                send_file(temp,sock);
                printf("Sent\n");
                
            }

            else if(buffer[length-1]=='1')
            {
                printf("1 achieved\n");
                 while(token!=NULL)
                    {
                        if(count==1)
                        {
                            strcpy(temp,token);
                        }
                        if(count==2)
                        {
                            strcpy(flag,token);
                        }

                        token = strtok(NULL, " ");
                        count++;
                    }

                // char *lastSlash = strrchr(token, '/');
                //  const char *myString = "path/to/some/file.txt/bitch";
                // printf("temp: %s\n",temp);
                if(containsChar(temp,'/'))
                {
                    const char *lastSlash = strrchr(temp, '/');
                    strcat(flag,lastSlash);
                }
                else
                {
                    strcat(flag,"/");
                    strcat(flag,temp);
                }
                printf("FLag: %s\n",flag);
                
                // strcat(flag,"/product.txt");

                 receive_and_create_file(flag,sock);   
                 printf("Received\n");
            }

            // else if(buffer[length-1]=='2')
            // {
            //     printf("2 achieved\n");
            //     while(token!=NULL)
            //         {
            //             if(count==1)
            //             {
            //                 strcpy(temp,token);
            //             }
            //             if(count==2)
            //             {
            //                 strcpy(flag,token);
            //             }

            //             token = strtok(NULL, " ");
            //             count++;
            //         }

            //     if(containsChar(temp,'/'))
            //     {
            //         const char *lastSlash = strrchr(temp, '/');
            //         strcat(flag,lastSlash);
            //     }
            //     else
            //     {
            //         strcat(flag,"/");
            //         strcat(flag,temp);
            //     }

            //      FILE *file = fopen(filename, "rb");
            //     if (!file) {
            //         perror("Error opening file for reading");
            //         exit(1);
            //     }

            //     FILE * file1=fopen(filename, "wb");

            //     if (!file) {
            //         perror("Error creating file");
            //         exit(1);
            //     }



            //     char buffer[MAX_CLIENT_PATH_LENGTH];
            //     size_t bytesRead;

            //     // while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            //     //     if (send(client_socket, buffer, bytesRead, 0) == -1) {
            //     //         perror("Error sending file");
            //     //         exit(1);
            //     //     }
            //     // }
            //     bzero(buffer,sizeof(buffer));
            //     bytesRead = fread(buffer, 1, sizeof(buffer), file);
            //     // send(client_socket, buffer, bytesRead, 0);

            //     fclose(file);
            // }
        }
    }

}



int main() 
{
 pthread_t server_thread, client_thread;

    // Create a thread for handling server connections
    if (pthread_create(&server_thread, NULL, handle_connections_server, (void *)&server_port) < 0) {
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


// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/stat.h>
// #include <dirent.h>
// #include <pthread.h>
// #include <semaphore.h>

// #define SERVER_IP "127.0.0.1"
// #define SERVER_PORT 8080
// #define CLIENT_PORT 8081
// #define MAX_PATH_LENGTH 1024
// #define MAX_PATHS 128
// int server_port=SERVER_PORT;
// int client_port=CLIENT_PORT;
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// #define MAX_CLIENT_PATH_LENGTH 4096
// int sock=0;
// // int flag=0;
// int flag[4096];
// sem_t mySemaphore;

// int jenkins_hash(const char *key) {
//     int hash = 0;

//     while (*key) {
//         hash += *key++;
//         hash += (hash << 10);
//         hash ^= (hash >> 6);
//     }

//     hash += (hash << 3);
//     hash ^= (hash >> 11);
//     hash += (hash << 15);

//     return hash;
// }

// void listFilesRecursive(const char *dirPath, const char *basePath, char (*paths)[MAX_PATH_LENGTH], int *count)
// {
//     DIR *dir;
//     struct dirent *entry;

//     // Open the directory
//     dir = opendir(dirPath);
//     if (!dir)
//     {
//         perror("opendir");
//         exit(EXIT_FAILURE);
//     }

//     // Loop through each entry in the directory
//     while ((entry = readdir(dir)) != NULL && *count < MAX_PATHS)
//     {
//         // Ignore "." and ".." entries
//         if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
//         {
//             // Construct full path of the current entry
//             char path[MAX_PATH_LENGTH];
//             snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);

//             // printf("%s\n",path);

//             // Store the relative path of the file with a forward slash
//             // prefixed at the beginning of the relative path
//             snprintf(paths[(*count)++], MAX_PATH_LENGTH, "%s", path + strlen(basePath) + 1);

//             // Check if the entry is a directory
//             if (entry->d_type == DT_DIR)
//             {
//                 // Recursively list files in the subdirectory
//                 listFilesRecursive(path, basePath, paths, count);
//             }
//         }
//     }

//     // Close the directory
//     closedir(dir);
// }

// void removeLastCharacter(char *str) {
//     size_t len = strlen(str);

//     // Check if the string is not empty
//     if (len > 0) {
//         // Set the last character to null terminator
//         str[len - 1] = '\0';
//     }
// }

// void removeDirectoryRecursively(const char *path) {
//     DIR *dir = opendir(path);
//     struct dirent *entry;

//     if (dir == NULL) {
//         perror("Error opening directory");
//         return;
//     }

//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
//             continue; // Skip the "." and ".." entries
//         }

//         char entryPath[PATH_MAX];
//         snprintf(entryPath, sizeof(entryPath), "%s/%s", path, entry->d_name);

//         if (entry->d_type == DT_DIR) {
//             // Recursive call for subdirectories
//             removeDirectoryRecursively(entryPath);
//         } else {
//             // Remove regular files
//             if (remove(entryPath) != 0) {
//                 perror("Error removing file");
//             }
//         }
//     }

//     closedir(dir);

//     // Remove the empty directory itself after its contents are removed
//     if (rmdir(path) != 0) {
//         perror("Error removing directory");
//     }
// }

// void* handle_operations_client(void* arg)
// {
//     int new_socket=*((int *)arg);
//     char buffer[2048];

//     bzero(buffer,sizeof(buffer));
//             recv(new_socket, buffer, sizeof(buffer),0);
//             printf("Command:%s\n",buffer);
//             char ack[] = "Acknowledgment: Data received";

//             if(strncmp(buffer,"READ",4)==0)
//             {
//                 char* token;
//                 int count = 0;

//                 token = strtok(buffer, " ");

//                 char temp[MAX_CLIENT_PATH_LENGTH];

//                 while(token!=NULL)
//                 {
//                     if(count==1)
//                     {
//                         strcpy(temp,token);
//                     }

//                     token = strtok(NULL, " ");
//                     count++;
//                 }

//                 char temp1[MAX_CLIENT_PATH_LENGTH];
//                 getcwd(temp1, sizeof(temp1));
//                 strcat(temp1,"/");
//                 strcat(temp1,temp);

//                 FILE* file=fopen(temp1,"rb");
//                 if (file == NULL) 
//                 {
//                     perror("Error opening file");
//                 }

//                 char bet_buf[1000];
//                 size_t bytesRead;

//                 while ((bytesRead = fread(bet_buf, 1, sizeof(bet_buf), file)) > 0) 
//                 {   
//                     fwrite(bet_buf, 1, bytesRead, stdout);
//                     send(new_socket,bet_buf,sizeof(bet_buf),0);
//                     bzero(bet_buf,sizeof(bet_buf));
//                     recv(new_socket, bet_buf, sizeof(bet_buf),0);

//                     // if(strcmp(bet_buf,"GOT")!=0)
//                     // {
//                     //     break;
//                     // }
//                 }
//                 bzero(bet_buf,sizeof(bet_buf));
//                 strcpy(bet_buf,"STOP");
//                 send(new_socket,bet_buf,sizeof(bet_buf),0);
//                 fclose(file);

//             }

//             else if(strncmp(buffer,"WRITE",5)==0)
//             {
//                 // pthread_mutex_lock(&mutex);
//                 sem_wait(&mySemaphore);
//                 if(flag[jenkins_hash(buffer)]==0)
//                 {
//                     flag[jenkins_hash(buffer)]=1;
//                     // pthread_mutex_unlock(&mutex);
//                     sem_post(&mySemaphore);
//                     sleep(10);
//                     char* token;
//                     int count = 0;

//                     token = strtok(buffer, " ");

//                     char temp[MAX_CLIENT_PATH_LENGTH];
//                     char to_write[MAX_CLIENT_PATH_LENGTH];
//                     bzero(temp,sizeof(temp));
//                     bzero(to_write,sizeof(to_write));

//                     while(token!=NULL)
//                     {
//                         if(count==1)
//                         {
//                             strcpy(temp,token);
//                         }
//                         if(count>=2)
//                         {
//                             strcat(to_write,token);
//                             strcat(to_write," ");
//                         }

//                         token = strtok(NULL, " ");
//                         count++;
//                     }
//                     removeLastCharacter(to_write);

//                     char temp1[MAX_CLIENT_PATH_LENGTH];
//                     getcwd(temp1, sizeof(temp1));
//                     strcat(temp1,"/");
//                     strcat(temp1,temp);

//                     FILE* file=fopen(temp1,"a");
//                     if (file == NULL) 
//                     {
//                         perror("Error opening file");
//                     }


//                     if (fputs(to_write, file) == EOF) 
//                     {
//                         perror("Error appending data to file");
//                         fclose(file);
//                     }

//                     bzero(to_write,sizeof(to_write));
//                     strcpy(to_write,"STOP");
//                     send(new_socket,to_write,sizeof(to_write),0);
//                     fclose(file);
                    
//                     flag[jenkins_hash(buffer)]=0;
//                     // pthread_mutex_unlock(&mutex);
//                 }
//                 else
//                 {
//                     printf("Failed\n");
//                     char to_write[MAX_CLIENT_PATH_LENGTH];
//                     bzero(to_write,sizeof(to_write));
//                     strcpy(to_write,"STOP");
//                     send(new_socket,to_write,sizeof(to_write),0);
//                     sem_post(&mySemaphore);
//                 }
//             }

//             else if(strncmp(buffer,"GETINFO",7)==0)
//             {
//                 char* token;
//                 int count = 0;

//                 token = strtok(buffer, " ");

//                 char temp[MAX_CLIENT_PATH_LENGTH];
//                 char to_write[MAX_CLIENT_PATH_LENGTH];

//                 while(token!=NULL)
//                 {
//                     if(count==1)
//                     {
//                         strcpy(temp,token);
//                     }

//                     token = strtok(NULL, " ");
//                     count++;
//                 }

//                 char temp1[MAX_CLIENT_PATH_LENGTH];
//                 getcwd(temp1, sizeof(temp1));
//                 strcat(temp1,"/");
//                 strcat(temp1,temp);

//                  struct stat file_info;

//                 // Get file information using stat
//                 if (stat(temp1, &file_info) != 0) {
//                     perror("Error getting file information");
//                 }

//                 snprintf(to_write, sizeof(to_write), "Size: %lld Permissions: %o", (long long)file_info.st_size,(unsigned int)(file_info.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)));
//                 send(new_socket,to_write,sizeof(to_write),0);
//                 bzero(to_write,sizeof(to_write));
//                 recv(new_socket, to_write, sizeof(to_write),0);

//                 bzero(to_write,sizeof(to_write));
//                 strcpy(to_write,"STOP");
//                 send(new_socket,to_write,sizeof(to_write),0);
//             }


//         close(new_socket);
//         pthread_exit(NULL);
// }

// void *handle_connections_client(void*arg)
// {
//     int port = *((int *)arg);

//     int server_fd, new_socket;
//     struct sockaddr_in address;
//     int addrlen = sizeof(address);

//     // Create socket
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//         perror("Socket creation failed");
//         exit(EXIT_FAILURE);
//     }

//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons(port);

//     // Bind the socket
//     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
//         perror("Bind failed");
//         exit(EXIT_FAILURE);
//     }

//     // Listen for incoming connections
//     if (listen(server_fd, 5) < 0) {
//         perror("Listen failed");
//         exit(EXIT_FAILURE);
//     }

//     char buffer[2048];

//      while (1) 
//     {
//         // Accept a new connection
//         if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
//             perror("Accept failed");
//             exit(EXIT_FAILURE);
//         }

//         // Find an available slot in the array
//         pthread_mutex_lock(&mutex);

//         pthread_t thread;

//         if (pthread_create(&thread, NULL, handle_operations_client, (int*)&new_socket) < 0) 
//         {
//             perror("Thread creation failed");
//             exit(EXIT_FAILURE);
//         }

//         pthread_mutex_unlock(&mutex);
//     }

// }

// void * handle_connections_server(void* arg)
// {
//     int port=*((int *)arg);
//     struct sockaddr_in serv_addr;
//     char buffer[4096];

//     // Create socket
//     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//         perror("Socket creation failed");
//         exit(EXIT_FAILURE);
//     }

//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(port);

//     // Convert IP address from string to binary form
//     if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
//         perror("Invalid address/ Address not supported");
//         exit(EXIT_FAILURE);
//     }

//     // Connect to the server
//     if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
//         perror("Connection failed");
//         exit(EXIT_FAILURE);
//     }

    

//      char currentDir[1024];
//     if (getcwd(currentDir, sizeof(currentDir)) == NULL) {
//         perror("getcwd");
//         exit(EXIT_FAILURE);
//     }


    
//      // Create an array to store paths
//     char paths[MAX_PATHS][MAX_PATH_LENGTH];
//     int num_paths = 0;

//     // List files in the current directory
//     listFilesRecursive(currentDir, currentDir, paths, &num_paths);

//     // Create a buffer to hold the data to be sent
//     char data[4096]; // Adjust the size as needed

//     // Format the data string with SERVER_IP, SERVER_PORT, CLIENT_PORT, and paths
//     snprintf(data, sizeof(data), "%s %d %d %d ", SERVER_IP, SERVER_PORT, CLIENT_PORT, num_paths);

//     // Append the paths to the data string
//     for (int i = 0; i < num_paths; ++i) {
//         strncat(data, paths[i], sizeof(data) - strlen(data) - 1);
//         strncat(data, " ", sizeof(data) - strlen(data) - 1);
//     }

//     // Send the data to the server
//     send(sock, data, sizeof(data), 0);
//     printf("Data sent to the server: %s\n", data);

//     // Receive the server's response
//     recv(sock, buffer, sizeof(buffer),0);
//     printf("Server response: %s\n", buffer);
//     printf("Socket: %d\n",sock);

//     while(1)
//     {
//         recv(sock, buffer, sizeof(buffer),0);
//         printf("Command: %s\n", buffer);

//         char *token = strtok(buffer, " ");
//         char temp[MAX_CLIENT_PATH_LENGTH];
//         int count=0;
//         char flag[3];

//         if(strncmp(buffer,"DELETE",6)==0)
//         {
//             while(token!=NULL)
//                     {
//                         if(count==1)
//                         {
//                             strcpy(temp,token);
//                         }
//                         if(count==2)
//                         {
//                             strcpy(flag,token);
//                         }

//                         token = strtok(NULL, " ");
//                         count++;
//                     }
            
//             char temp1[MAX_CLIENT_PATH_LENGTH];
//             getcwd(temp1, sizeof(temp1));
//             strcat(temp1,"/");
//             strcat(temp1,temp);

//             if(strcmp(flag,"-f")==0)
//             {
//                 remove(temp1);
//             }

//             else if(strcmp(flag,"-d")==0)
//             {
//                 removeDirectoryRecursively(temp1);
//             }

           
//            for (int i = 0; i < num_paths; i++) {
//         if (strcmp(paths[i], temp1) == 0) {
//         // Path found, remove it from paths
        

//         // Shift the remaining elements in the array to fill the gap
//         for (int j = i; j < num_paths - 1; j++) {
//             strcpy(paths[j], paths[j + 1]);
//         }

//         // Decrease the number of paths in the array
//         num_paths--;

//         break;
//       }
//    }


//             char ack[1024];
//             strcpy(ack,"STOP");

//             send(sock,ack,sizeof(ack),0);
//         }

//         else if(strncmp(buffer,"CREATE",6)==0)
//         {
//             while(token!=NULL)
//                     {
//                         if(count==1)
//                         {
//                             strcpy(temp,token);
//                         }
//                         if(count==2)
//                         {
//                             strcpy(flag,token);
//                         }

//                         token = strtok(NULL, " ");
//                         count++;
//                     }
            
//             char temp1[MAX_CLIENT_PATH_LENGTH];
//             getcwd(temp1, sizeof(temp1));
//             strcat(temp1,"/");
//             strcat(temp1,temp);

//             if(strcmp(flag,"-d")==0)
//             {
//                 mkdir(temp1,S_IRWXU);
//             }
            
//             else if(strcmp(flag,"-f")==0)
//             {
//                 FILE* file=fopen(temp1,"w");
//                 fclose(file);
//             }

//             char ack[1024];
//             strcpy(ack,"STOP");

//             send(sock,ack,sizeof(ack),0);
//         }
//     }

// }



// int main() 
// {
//     sem_init(&mySemaphore, 0, 1);
//  pthread_t server_thread, client_thread;

//     // Create a thread for handling server connections
//     if (pthread_create(&server_thread, NULL, handle_connections_server, (void *)&server_port) < 0) {
//         perror("Server thread creation failed");
//         exit(EXIT_FAILURE);
//     }

//     // Create a thread for handling client connections
//     if (pthread_create(&client_thread, NULL, handle_connections_client, (void *)&client_port) < 0) {
//         perror("Client thread creation failed");
//         exit(EXIT_FAILURE);
//     }

//     // Wait for threads to finish (this won't happen as they run in an infinite loop)
//     pthread_join(server_thread, NULL);
//     pthread_join(client_thread, NULL);

//     return 0;
// }
