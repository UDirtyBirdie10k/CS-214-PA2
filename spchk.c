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
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    int capacity = 1000;
    int count = 0;


    char **words = (char **)malloc(capacity * sizeof(char *));
    if (words == NULL) {
        perror("Memory allocation failed");
        close(file);
        return NULL;
    }

    while ((bytesRead = read(file, buffer, BUFFER_SIZE)) > 0) {
        char *ptr = buffer;
        char *word;
        while ((word = strtok(ptr, " \t\n")) != NULL) { // Split buffer into words
            count++;
            ptr = NULL;
            words = realloc(words, count * sizeof(char*));
            if (words == NULL) {
                perror("Error allocating memory");
                close(file);
                return NULL; // Return NULL if memory allocation fails
            }
            words[count - 1] = strdup(word); // Store the word in the array
            if (words[count - 1] == NULL) {
                perror("Error duplicating string");
                close(file);
                return NULL; // Return NULL if string duplication fails
            }
        }
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
           // printf("word found\n");
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
    //printf("word not found");
    return 0;
}

void spell_check(const char *filename, char *dictionary[], int wordCount) {
    int file = open(filename, O_RDONLY);
    if (file == -1) {
        perror("Error opening file");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    int line_number = 1;
    int char_number = 1;
    char *word = NULL;
    int word_length = 0;

    // Read characters from the file
    while ((bytesRead = read(file, buffer, BUFFER_SIZE)) > 0) {
        
        for (ssize_t i = 0; i < bytesRead; i++) {
            char ch = buffer[i];
            // Check for newline character
            if (ch == '\n') {
                // Increment line number and reset character number
                line_number++;
                char_number = 1;
            } if (ch == ' ' || ch == '\t' || ch == '\r' || ch=='\n') {
                // Word boundary found, store the word if it's not empty
                if (word_length > 0) {
                    // Allocate memory for the word and copy it
                    word = (char *)malloc((word_length + 1) * sizeof(char));
                                printf("%s\n", word);

                    if (word == NULL) {
                        perror("Memory allocation error");
                        close(file);
                        return;
                    }
                    strncpy(word, &buffer[i - word_length], word_length);
                    word[word_length] = '\0'; // Null-terminate the string
                    // Print the word along with line and character number
                    if (binarySearch(dictionary, wordCount, word) == 0) {
                        printf("%s (%d, %d): %s\n", filename, line_number, char_number - word_length, word);
                    }
                    // Free allocated memory
                    free(word);
                    word_length = 0; // Reset word length
                }
            } else {
                // Non-space character, increment character number
                char_number++;
                word_length++; // Increment word length
            }
        }
    }

    if (bytesRead == -1) {
        perror("Error reading from file");
    }

    // Close the file
    close(file);
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


    for (int i = 0; i < wordCount - 1; i++) {
       //printf("%s\n", words[i]);
    }
    if (words == NULL) {
        fprintf(stderr, "Failed to create dictionary\n");
        return EXIT_FAILURE;
    }

    spell_check(testfile, words, wordCount - 1);

    // Free allocated memory
    for (int i = 0; i < wordCount; i++) {
        free(words[i]);
    }
    free(words);

    return EXIT_SUCCESS;
}
