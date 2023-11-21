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

void compress_and_send_directory(const char *dirname, int client_socket) {
    char command[256];
    snprintf(command, sizeof(command), "tar czf - %s", dirname);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        perror("Error opening pipe for compression");
        exit(1);
    }

    char buffer[MAX_BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        if (send(client_socket, buffer, bytesRead, 0) == -1) {
            perror("Error sending compressed directory");
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

    // Directory to be sent
    const char *dirname = "example_directory";

    // Compress and send the directory
    compress_and_send_directory(dirname, client_socket);

    // Close sockets
    close(client_socket);
    close(server_socket);

    return 0;
}
