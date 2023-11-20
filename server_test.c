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

// void compress_and_send_directory(const char *dirname, int client_socket) {
//     struct archive *a = archive_write_new();
//     archive_write_add_filter_gzip(a);
//     archive_write_set_format_pax_restricted(a);
//     archive_write_open_fd(a, client_socket);

//     struct archive *disk = archive_read_disk_new();
//     archive_read_disk_set_standard_lookup(disk);

//     struct archive_entry *entry;

//     if (archive_read_disk_open(disk, dirname) != ARCHIVE_OK) {
//         fprintf(stderr, "Error opening disk archive\n");
//         exit(1);
//     }

//     while (archive_read_next_header(disk, &entry) == ARCHIVE_OK) {
//         struct archive_entry *clone = archive_entry_clone(entry);
//         archive_write_header(a, clone);
//         archive_entry_free(clone);

//         const void *buff;
//         size_t size;
//         la_int64_t offset;

//         while (archive_read_data_block(disk, &buff, &size, &offset) == ARCHIVE_OK) {
//             archive_write_data(a, buff, size);
//         }
//     }

//     archive_read_close(disk);
//     archive_read_free(disk);

//     // Use archive_write_close and archive_write_free instead of archive_write_finish
//     archive_write_close(a);
//     archive_write_free(a);
// }

// int main() {
//     int server_socket, client_socket;
//     socklen_t client_length;
//     struct sockaddr_in server_address, client_address;

//     // Create socket
//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket < 0) {
//         error("Error opening socket");
//     }

//     // Initialize server address
//     memset((char *)&server_address, 0, sizeof(server_address));
//     server_address.sin_family = AF_INET;
//     server_address.sin_addr.s_addr = INADDR_ANY;
//     server_address.sin_port = htons(PORT);

//     // Bind the socket to the server address
//     if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
//         error("Error on binding");
//     }

//     // Listen for incoming connections
//     listen(server_socket, 5);

//     printf("Server listening on port %d...\n", PORT);

//     client_length = sizeof(client_address);

//     // Accept connection from the client
//     client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_length);
//     if (client_socket < 0) {
//         error("Error on accept");
//     }

//     printf("Client connected...\n");

//     // Directory to be sent
//     const char *dirname = "example_directory";

//     // Compress and send the directory
//     compress_and_send_directory(dirname, client_socket);

//     // Close sockets
//     close(client_socket);
//     close(server_socket);

//     return 0;
// }


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

// void compress_and_send_file(const char *filename, int client_socket) {
//     struct archive *a = archive_write_new();
//     archive_write_add_filter_gzip(a);
//     archive_write_set_format_pax_restricted(a);
//     archive_write_open_fd(a, client_socket);

//     struct archive_entry *entry = archive_entry_new();

//     FILE *file = fopen(filename, "rb");
//     if (!file) {
//         fprintf(stderr, "Error opening file %s\n", filename);
//         exit(1);
//     }

//     struct stat file_stat;
//     if (fstat(fileno(file), &file_stat) != 0) {
//         perror("Error getting file stat");
//         exit(1);
//     }

//     archive_entry_copy_stat(entry, &file_stat);
//     archive_entry_copy_pathname(entry, filename);

//     archive_write_header(a, entry);

//     char buffer[MAX_BUFFER_SIZE];
//     size_t bytesRead;

//     while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
//         archive_write_data(a, buffer, bytesRead);
//     }

//     fclose(file);

//     archive_entry_free(entry);

//     // Use archive_write_close and archive_write_free instead of archive_write_finish
//     archive_write_close(a);
//     archive_write_free(a);
// }

// int main() {
//    int server_socket, client_socket;
//     socklen_t client_length;
//     struct sockaddr_in server_address, client_address;

//     // Create socket
//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket < 0) {
//         error("Error opening socket");
//     }

//     // Initialize server address
//     memset((char *)&server_address, 0, sizeof(server_address));
//     server_address.sin_family = AF_INET;
//     server_address.sin_addr.s_addr = INADDR_ANY;
//     server_address.sin_port = htons(PORT);

//     // Bind the socket to the server address
//     if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
//         error("Error on binding");
//     }

//     // Listen for incoming connections
//     listen(server_socket, 5);

//     printf("Server listening on port %d...\n", PORT);

//     client_length = sizeof(client_address);

//     // Accept connection from the client
//     client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_length);
//     if (client_socket < 0) {
//         error("Error on accept");
//     }

//     printf("Client connected...\n");

//     // File to be sent
//     const char *filename = "test.txt";

//     // Compress and send the file
//     compress_and_send_file(filename, client_socket);

//     // Close sockets
//     close(client_socket);
//     close(server_socket);

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

void compress_and_send_file(const char *filename, int client_socket) {
    char command[256];
    snprintf(command, sizeof(command), "tar czf - %s", filename);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        perror("Error opening pipe for compression");
        exit(1);
    }

    char buffer[MAX_BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        if (send(client_socket, buffer, bytesRead, 0) == -1) {
            perror("Error sending compressed file");
            exit(1);
        }
    }

    pclose(pipe);
}

int main() {
    int server_socket, client_socket;
    socklen_t client_length;
    struct sockaddr_in server_address, client_address;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        error("Error opening socket");
    }

    // Initialize server address
    memset((char *)&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        error("Error on binding");
    }

    // Listen for incoming connections
    listen(server_socket, 5);

    printf("Server listening on port %d...\n", PORT);

    client_length = sizeof(client_address);

    // Accept connection from the client
    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_length);
    if (client_socket < 0) {
        error("Error on accept");
    }

    printf("Client connected...\n");

    // File to be sent
    const char *filename = "test.txt";

    // Compress and send the file
    compress_and_send_file(filename, client_socket);

    // Close sockets
    close(client_socket);
    close(server_socket);

    return 0;
}
