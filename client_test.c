// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <archive.h>
// #include <archive_entry.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>

// #define PORT 8080
// #define MAX_BUFFER_SIZE 1024

// void error(const char *msg) {
//     perror(msg);
//     exit(1);
// }

// void receive_and_extract_directory(const char *dirname, int server_socket) {
//     struct archive *a = archive_read_new();
//     archive_read_support_filter_gzip(a);
//     archive_read_support_format_all(a);

//     int r = archive_read_open_fd(a, server_socket, MAX_BUFFER_SIZE);
//     if (r != ARCHIVE_OK) {
//         fprintf(stderr, "Error opening archive for reading\n");
//         exit(1);
//     }

//     struct archive *disk = archive_write_disk_new();
//     archive_write_disk_set_options(disk, ARCHIVE_EXTRACT_TIME);
//     archive_write_disk_set_standard_lookup(disk);

//     for (;;) {
//         struct archive_entry *entry;
//         r = archive_read_next_header(a, &entry);
//         if (r == ARCHIVE_EOF) {
//             break;
//         }

//         if (r != ARCHIVE_OK) {
//             fprintf(stderr, "Error reading archive header\n");
//             exit(1);
//         }

//         // Get the entry path and replicate it in the current directory
//         const char *entry_path = archive_entry_pathname(entry);
//         char replicated_path[256];
//         snprintf(replicated_path, sizeof(replicated_path), "./%s", entry_path);

//         // Check if the entry is a directory and create it in the current directory
//         if (archive_entry_filetype(entry) == AE_IFDIR) {
//             mkdir(replicated_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
//         } else {
//             // If it's a file, create and write the file in the current directory
//             r = archive_write_header(disk, entry);
//             if (r != ARCHIVE_OK) {
//                 fprintf(stderr, "Error writing archive header\n");
//                 exit(1);
//             }

//             for (;;) {
//                 const void *buff;
//                 size_t size;
//                 la_int64_t offset;

//                 r = archive_read_data_block(a, &buff, &size, &offset);
//                 if (r == ARCHIVE_EOF) {
//                     break;
//                 }

//                 if (r != ARCHIVE_OK) {
//                     fprintf(stderr, "Error reading archive data block\n");
//                     exit(1);
//                 }

//                 // Write the data block to the replicated file
//                 r = archive_write_data_block(disk, buff, size, offset);
//                 if (r != ARCHIVE_OK) {
//                     fprintf(stderr, "Error writing archive data block\n");
//                     exit(1);
//                 }
//             }
//         }
//     }

//     archive_read_close(a);
//     archive_read_free(a);

//     archive_write_close(disk);
//     archive_write_free(disk);
// }

// int main() {
//     int client_socket;
//     struct sockaddr_in server_address;

//     // Create socket
//     client_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (client_socket < 0) {
//         error("Error opening socket");
//     }

//     // Initialize server address
//     memset((char *)&server_address, 0, sizeof(server_address));
//     server_address.sin_family = AF_INET;
//     server_address.sin_addr.s_addr = INADDR_ANY;
//     server_address.sin_port = htons(PORT);

//     // Connect to the server
//     if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
//         error("Error connecting to server");
//     }

//     printf("Connected to server...\n");

//     // Directory to receive and extract
//     const char *dirname = "received_directory";

//     // Receive and extract the directory
//     receive_and_extract_directory(dirname, client_socket);

//     // Close socket
//     close(client_socket);

//     return 0;
// }


// void receive_and_extract_file(const char *filename, int server_socket) {
//     struct archive *a = archive_read_new();
//     archive_read_support_filter_gzip(a);
//     archive_read_support_format_all(a);

//     int r = archive_read_open_fd(a, server_socket, MAX_BUFFER_SIZE);
//     if (r != ARCHIVE_OK) {
//         fprintf(stderr, "Error opening archive for reading\n");
//         exit(1);
//     }

//     struct archive *disk = archive_write_disk_new();
//     archive_write_disk_set_options(disk, ARCHIVE_EXTRACT_TIME);
//     archive_write_disk_set_standard_lookup(disk);

//     struct archive_entry *entry;
//     r = archive_read_next_header(a, &entry);

//     if (r != ARCHIVE_OK) {
//         fprintf(stderr, "Error reading archive header\n");
//         exit(1);
//     }

//     // Get the entry path and replicate it in the current directory
//     const char *entry_path = archive_entry_pathname(entry);
//     char replicated_path[256];
//     snprintf(replicated_path, sizeof(replicated_path), "./%s", entry_path);

//     // If it's a file, create and write the file in the current directory
//     FILE *file = fopen(replicated_path, "wb");
//     if (!file) {
//         fprintf(stderr, "Error creating file %s\n", replicated_path);
//         exit(1);
//     }

//     for (;;) {
//         const void *buff;
//         size_t size;
//         la_int64_t offset;

//         r = archive_read_data_block(a, &buff, &size, &offset);
//         if (r == ARCHIVE_EOF) {
//             break;
//         }

//         if (r != ARCHIVE_OK) {
//             fprintf(stderr, "Error reading archive data block\n");
//             exit(1);
//         }

//         // Write the data block to the replicated file
//         fwrite(buff, 1, size, file);
//     }

//     fclose(file);

//     archive_read_close(a);
//     archive_read_free(a);

//     archive_write_close(disk);
//     archive_write_free(disk);
// }


// int main() {
//     int client_socket;
//     struct sockaddr_in server_address;

//     // Create socket
//     client_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (client_socket < 0) {
//         error("Error opening socket");
//     }

//     // Initialize server address
//     memset((char *)&server_address, 0, sizeof(server_address));
//     server_address.sin_family = AF_INET;
//     server_address.sin_addr.s_addr = INADDR_ANY;
//     server_address.sin_port = htons(PORT);

//     // Connect to the server
//     if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
//         error("Error connecting to server");
//     }

//     printf("Connected to server...\n");

//     // File to receive and create
//     const char *filename = "test.txt";

//     // Receive and extract the file
//     receive_and_extract_file(filename, client_socket);

//     // Close socket
//     close(client_socket);

//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void receive_and_extract_file(const char *filename, int server_socket) {
    char command[256];
    snprintf(command, sizeof(command), "tar xzf - -C .");

    FILE *pipe = popen(command, "w");
    if (!pipe) {
        perror("Error opening pipe for extraction");
        exit(1);
    }

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytesRead, pipe);
    }

    pclose(pipe);
}

int main() {
    int client_socket;
    struct sockaddr_in server_address;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        error("Error opening socket");
    }

    // Initialize server address
    memset((char *)&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        error("Error connecting to server");
    }

    printf("Connected to server...\n");

    // File to receive and create
    const char *filename = "test.txt";

    // Receive and extract the file
    receive_and_extract_file(filename, client_socket);

    // Close socket
    close(client_socket);

    return 0;
}
