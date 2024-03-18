#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define maxWordLength 50
#define BUFFER_SIZE 1024

char **reAddDictionary(const char *filename, int *wordCount) {
    int file = open(filename, O_RDONLY);
    if (file == -1) {
        perror("Unable to open file");
        return NULL;
    }

    int capacity = 1000;
    int count = 0;
    char **words = (char **)malloc(capacity * sizeof(char *));
    if (words == NULL) {
        perror("Memory allocation failed");
        close(file);
        return NULL;
    }

    char word[maxWordLength];
    ssize_t bytesRead;
    while ((bytesRead = read(file, word, maxWordLength - 1)) > 0) {
        // Null-terminate the word
        word[bytesRead] = '\0';
        
        if (count >= capacity) {
            capacity *= 2;
            char **temp = realloc(words, capacity * sizeof(char *));
            if (temp == NULL) {
                perror("Memory reallocation failed");
                close(file);
                for (int i = 0; i < count; i++) {
                    free(words[i]);
                }
                free(words);
                return NULL;
            }
            words = temp;
        }

        words[count] = strdup(word);
        if (words[count] == NULL) {
            perror("Memory allocation failed");
            close(file);
            for (int i = 0; i < count; i++) {
                free(words[i]);
            }
            free(words);
            return NULL;
        }
        count++;
    }

    if (bytesRead == -1) {
        perror("Error reading from file");
        close(file);
        for (int i = 0; i < count; i++) {
            free(words[i]);
        }
        free(words);
        return NULL;
    }

    close(file);
    *wordCount = count;
    return words;
}

int binarySearch(char *dictionary[], int size, const char *word) {
    int low = 0;
    int high = size - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        int comparison = strcmp(dictionary[mid], word);
        if (comparison == 0) {
            // Word found in the dictionary
            printf("word found");
            return 1;
        } else if (comparison < 0) {
            // Word may be in the right half
            low = mid + 1;
        } else {
            // Word may be in the left half
            high = mid - 1;
        }
    }

    // Word not found in the dictionary
    printf("word not found");
    return 0;
}

void spell_check(const char *filename, char *dictionary[], int wordCount) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    int lineNumber = 1;
    int charNumber = 1;
    char *word = NULL;
    int wordLength = 0;

    // Read characters from the file
    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) {
            char ch = buffer[i];
            // Check for newline character
            if (ch == '\n') {
                // Increment line number and reset character number
                lineNumber++;
                charNumber = 1;
            } else if (isspace(ch)) {
                // Word boundary found, store the word if it's not empty
                if (wordLength > 0) {
                    // Allocate memory for the word and copy it
                    word = (char *)malloc((wordLength + 1) * sizeof(char));
                    if (word == NULL) {
                        perror("Memory allocation error");
                        close(fd);
                        return;
                    }
                    strncpy(word, &buffer[i - wordLength], wordLength);
                    word[wordLength] = '\0'; // Null-terminate the string
                    // Print the word along with line and character number
                    if (binarySearch(dictionary, wordCount, word) == 1) {
                        printf("%s (%d, %d): %s\n", filename, lineNumber, charNumber - wordLength, word);
                    }
                    // Free allocated memory
                    free(word);
                    wordLength = 0; // Reset word length
                }
            } else {
                // Non-space character, increment character number
                charNumber++;
                wordLength++; // Increment word length
            }
        }
    }

    if (bytesRead == -1) {
        perror("Error reading from file");
    }

    // Close the file
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <dictionary_file> <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* filename = "dict1.txt"; // Path to the file
    const char* testfile  = "test1.txt";

    int wordCount;
    char **words = reAddDictionary(filename, &wordCount);
    for (int i = 0; i < wordCount; i++) {
        printf(words[i]);
        printf(testfile);
    }
    if (words == NULL) {
        fprintf(stderr, "Failed to create dictionary\n");
        return EXIT_FAILURE;
    }

    spell_check(testfile, words, wordCount);

    // Free allocated memory
    for (int i = 0; i < wordCount; i++) {
        free(words[i]);
    }
    free(words);

    return EXIT_SUCCESS;
}
