#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#define MAX_PATH_LENGTH 1024
#define MAX_WORD_LENGTH 50

int is_txt_file(const char *filename) {
    size_t len = strlen(filename);
    return len > 4 && strcmp(filename + len - 4, ".txt") == 0;
}

void check_spelling(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return;
    }

    char word[MAX_WORD_LENGTH];
    int line_number = 1;
    while (fgets(word, sizeof(word), file) != NULL) {
        char *ptr = word;
        int col_number = 1;
        while (*ptr) {
            if (isalpha(*ptr)) {
                char word[MAX_WORD_LENGTH];
                int i = 0;
                while (isalpha(*ptr)) {
                    word[i++] = tolower(*ptr);
                    ptr++;
                }
                word[i] = '\0'; // Null-terminate the word
                // Here you can implement your spelling check logic
                // For demonstration purposes, let's consider all words as incorrect
                printf("%s (%d,%d): %s\n", filename, line_number, col_number, word);
            } else {
                if (*ptr == '\n') {
                    line_number++;
                    col_number = 0;
                }
                col_number++;
                ptr++;
            }
        }
        line_number++;
    }

    fclose(file);
}

void traverse_directory(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && is_txt_file(entry->d_name)) {
            char filepath[MAX_PATH_LENGTH];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirname, entry->d_name);
            check_spelling(filepath);
        } else if (entry->d_type == DT_DIR &&
                   strcmp(entry->d_name, ".") != 0 &&
                   strcmp(entry->d_name, "..") != 0 &&
                   entry->d_name[0] != '.') {
            char subdir[MAX_PATH_LENGTH];
            snprintf(subdir, sizeof(subdir), "%s/%s", dirname, entry->d_name);
            traverse_directory(subdir);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <dictionary> <file1> [<file2> ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 2; i < argc; i++) {
        check_spelling(argv[i]);
    }

    return EXIT_SUCCESS;
}