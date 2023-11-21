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

void receive_and_extract_directory(const char *dirname, int server_socket) {
    char command[256];
    snprintf(command, sizeof(command), "tar xzf - -C %s", dirname);

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

    // Directory to receive and extract
    const char *dirname = "received_directory";

    // Receive and extract the directory
    receive_and_extract_directory(dirname, client_socket);

    // Close socket
    close(client_socket);

    return 0;
}
