#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define maxPathLength 1024
#define maxWordLength 50

// Hardcoded dictionary of correctly spelled words
const char *dictionary[] = {
    "hello",
    "world",
    "example",
    "spelling",
    "check",
    // Add more words as needed
    NULL // NULL terminator
};

// Function to check if a word is spelled correctly
int is_spelled_correctly(const char *word, ) {
    for (int i = 0; dictionary[i] != NULL; i++) {
        if (strcmp(dictionary[i], word) == 0) {
            return 1; // Word found in dictionary
        }
    }
    return 0; // Word not found in dictionary
}

void spell_check(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    char word[maxWordLength];
    int line_number = 1;
    ssize_t bytes_read;
    while ((bytes_read = read(fd, word, sizeof(word))) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (!isalpha(word[i])) {
                if (word[i] == '\n') {
                    line_number++;
                }
                word[i] = '\0';
                if (!is_spelled_correctly(word)) {
                    printf("%s (%d,%d): %s\n", filename, line_number, i + 1, word);
                }
                word[0] = '\0';
            }
        }
    }

    if (bytes_read == -1) {
        perror("Error reading file");
    }

    close(fd);
}

void traverse_directory(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("failed opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        char filePath[maxPathLength];
        snprintf(filePath, maxPathLength, "%s/%s", dirname, entry->d_name);

        struct stat pathStat;
        if (stat(filePath, &pathStat) == 0) {
            if (S_ISDIR(pathStat.st_mode)) {
                traverse_directory(filePath);
            } else if (S_ISREG(pathStat.st_mode)) {
                char *dot = strrchr(entry->d_name, '.');
                if (dot != NULL && strcmp(dot, ".txt") == 0) {
                    spell_check(filePath);
                }
            }
        } else {
            perror("File status failed.");
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    traverse_directory(argv[1]);

    return EXIT_SUCCESS;
}
