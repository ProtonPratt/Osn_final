#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include <dirent.h>

void listFiles(const char *dirPath, const char *basePath) {
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir(dirPath);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // Loop through each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Ignore "." and ".." entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Construct full path of the current entry
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);

            // Check if the entry is a directory
            if (entry->d_type == DT_DIR) {
                // Recursively list files in the subdirectory
                listFiles(path, path);
            } else {
                // Print the relative path of the file
                printf("%s\n", path);
            }
        }
    }

    // Close the directory
    closedir(dir);
}

int main() {
    // Get the current working directory
    char currentDir[1024];
    if (getcwd(currentDir, sizeof(currentDir)) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    // List files in the current directory
    listFiles(currentDir, currentDir);

    return 0;
}
