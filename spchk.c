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

void reAddDictionary(const char *dirname) {
    // Open the directory
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }

    // Dynamically allocate memory for the array of words
    int capacity = 1000;
    char **words = (char **)malloc(capacity * sizeof(char *));
    if (words == NULL) {
        perror("Memory allocation failed");
        closedir(dir);
        return;
    }

    // Initialize count of words
    int count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry is a regular file
        if (entry->d_type == DT_REG) {
            // Construct the file path
            char filepath[maxPathLength];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirname, entry->d_name);

            // Open the file
            FILE *file = fopen(filepath, "r");
            if (file == NULL) {
                perror("Unable to open file");
                continue;
            }

            // Read words from the file
            char word[maxWordLength];
            while (fscanf(file, "%s", word) != EOF) {
                // Check if the array needs to be resized
                if (count >= capacity) {
                    capacity *= 2;
                    char **temp = (char **)realloc(words, capacity * sizeof(char *));
                    if (temp == NULL) {
                        perror("Memory reallocation failed");
                        fclose(file);
                        closedir(dir);
                        for (int i = 0; i < count; i++) {
                            free(words[i]);
                        }
                        free(words);
                        return;
                    }
                    words = temp;
                }

                // Allocate memory for the word and copy it
                words[count] = (char *)malloc((strlen(word) + 1) * sizeof(char));
                if (words[count] == NULL) {
                    perror("Memory allocation failed");
                    fclose(file);
                    closedir(dir);
                    for (int i = 0; i < count; i++) {
                        free(words[i]);
                    }
                    free(words);
                    return;
                }
                strcpy(words[count], word);
                count++;
            }

            // Close the file
            fclose(file);
        }
    }

    // Close the directory
    closedir(dir);

    // Output the words
    printf("Words read from files in directory '%s':\n", dirname);
    for (int i = 0; i < count; i++) {
        printf("%s\n", words[i]);
    }

    // Free memory
    for (int i = 0; i < count; i++) {
        free(words[i]);
    }
    free(words);
}


// Function to check if a word is spelled correctly
int is_spelled_correctly(const char *word, const char **dirname) {
    for (int i = 0; dirname[i] != NULL; i++) {
        if (strcmp(dirname[i], word) == 0) {
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
