#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

#define maxPathLength 1024
#define maxWordLength 50


void spell_check(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char word[maxWordLength];
    int line_number = 1;
    while (fgets(word, sizeof(word), file) != NULL) {
        char *ptr = word;
        int col_number = 1;
        while (*ptr) {
            if (isalpha(*ptr)) {
                char word[maxWordLength];
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
        if(stat(filePath, &pathStat) == 0){
            if(S_ISDIR(pathStat.st_mode)){
                traverse_directory(filePath);
            } else if(S_ISREG(pathStat.st_mode)){
                char *dot = strrchr(entry->d_name, '.');
                if(strcmp(dot, ".txt") ==0 ){
                    processFile(filePath);
                }
            }
        } else{
            perror("File status failed.");
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

