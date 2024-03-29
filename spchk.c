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
                                            
                                            //printf("%s\n", word);

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

int binarySearch(char *words[], int size, const char *word) {
    int left = 0;
    int right = size - 1;


    while (left <= right) {
        int mid = left + (right - left) / 2;
        int comparison = strcmp(words[mid], word);

        if (comparison == 0) {
           //printf("word found\n");
            return 1;
        } else if (comparison < 0) {
            // right half
            left = mid + 1;
        } else {
            //  left half
            right = mid - 1;
        }
    }

    //printf("word not found ");
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
    int line_number = 0;
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
            } if (ch == ' ' || ch == '\t' || ch=='\n') {
                
                if (word_length > 0) {
                    
                    word = (char *)malloc((word_length + 1) * sizeof(char));
                    if (word == NULL) {
                        perror("Memory allocation error");
                        close(file);
                        return;
                    }
                    strncpy(word, &buffer[i - word_length], word_length);
                    word[word_length] = '\0'; 
                    // Print the word along with line and character number
                    //convert word to lowercase
                    for (int j = 0; word[j]; j++){
                        word[j] = tolower(word[j]);
                    }                    
                    
                    //printf("%s\n", word);

                    if (binarySearch(dictionary, wordCount, word) == 0) {
                        
                        int word_start_char = char_number - word_length;
                        
                        printf("%s (%d, %d): %s\n", filename, line_number, char_number - word_length + 1, word);
                    }
                    free(word);
                    word_length = 0; // Reset word length
                }
            } else {
                
                char_number++;
                word_length++; 
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
