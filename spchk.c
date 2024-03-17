#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_PATH_LENGTH 1024
#define MAX_WORD_LENGTH 50
#define BUFFER_SIZE 1024

char **reAddDictionary(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("Unable to open directory");
        return NULL;
    }

    int capacity = 1000;
    char **words = (char **)malloc(capacity * sizeof(char *));
    if (words == NULL) {
        perror("Memory allocation failed");
        closedir(dir);
        return NULL;
    }

    int count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char filepath[MAX_PATH_LENGTH];
        snprintf(filepath, sizeof(filepath), "%s/%s", dirname, entry->d_name);

        FILE *file = fopen(filepath, "r");
        if (file == NULL) {
            perror("Unable to open file");
            continue;
        }

        char word[MAX_WORD_LENGTH];
        while (fscanf(file, "%s", word) != EOF) {
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
                    return NULL;
                }
                words = temp;
            }

            words[count] = (char *)malloc((strlen(word) + 1) * sizeof(char));
            if (words[count] == NULL) {
                perror("Memory allocation failed");
                fclose(file);
                closedir(dir);
                for (int i = 0; i < count; i++) {
                    free(words[i]);
                }
                free(words);
                return NULL;
            }
            strcpy(words[count], word);
            count++;
        }

        fclose(file);
    }

    printf("Words in the dictionary:\n");
    for (int i = 0; i < count; i++) {
        printf("%s\n", words[i]);
    }

    closedir(dir);

    return words;
}

int isSpelledCorrectly(const char *word, char **words, int wordCount) {
    for (int i = 0; i < wordCount; i++) {
        if (strcmp(words[i], word) == 0) {
            printf("Word found\n");
            return 1; // Word found in dictionary
        }
    }
    printf("Word not found\n");
    return 0; // Word not found in dictionary
}

void spellCheckFiles(const char *dirname, char **words, int wordCount) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char filePath[MAX_PATH_LENGTH];
        snprintf(filePath, MAX_PATH_LENGTH, "%s/%s", dirname, entry->d_name);

        struct stat pathStat;
        if (stat(filePath, &pathStat) == 0) {
            if (S_ISDIR(pathStat.st_mode)) {
                spellCheckFiles(filePath, words, wordCount);
            } else if (S_ISREG(pathStat.st_mode)) {
                FILE *file = fopen(filePath, "r");
                if (file == NULL) {
                    perror("Failed to open file");
                    continue;
                }

                char word[MAX_WORD_LENGTH];
                int lineNumber = 1;
                while (fscanf(file, "%s", word) != EOF) {
                    char *ptr = word;
                    while (*ptr) {
                        if (!isalpha(*ptr)) {
                            *ptr = '\0';
                            break;
                        }
                        ptr++;
                    }

                    if (!isSpelledCorrectly(word, words, wordCount)) {
                        printf("Misspelled word found: %s (File: %s, Line: %d)\n", word, entry->d_name, lineNumber);
                    }

                    lineNumber++;
                }

                fclose(file);
            }
        } else {
            perror("Failed to get file status");
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char **words = reAddDictionary(argv[1]);
    if (words == NULL) {
        fprintf(stderr, "Failed to load dictionary\n");
        return EXIT_FAILURE;
    }

    // Count the number of words
    int wordCount = 0;
    while (words[wordCount] != NULL) {
        wordCount++;
    }

    spellCheckFiles(argv[1], words, wordCount);

    // Free allocated memory
    for (int i = 0; i < wordCount; i++) {
        free(words[i]);
    }
    free(words);

    return EXIT_SUCCESS;
}
