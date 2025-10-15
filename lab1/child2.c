#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 1024
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        fprintf(stderr, "This program reads from stdin and processes spaces.\n");
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    char result[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("Child2 started. PID: %d\n", getpid());

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        
        char *src = buffer;
        char *dst = result;
        int space_count = 0;
        int in_space_sequence = 0;
        
        while (*src) {
            if (isspace((unsigned char)*src)) {
                if (!in_space_sequence) {
                    in_space_sequence = 1;
                    space_count = 1;
                } else {
                    space_count++;
                }
            } else {
                if (in_space_sequence) {
                    if (space_count % 2 != 0) {
                        *dst++ = ' ';
                    }
                    in_space_sequence = 0;
                    space_count = 0;
                }
                *dst++ = *src;
            }
            src++;
        }
        
        if (in_space_sequence && space_count % 2 != 0) {
            *dst++ = ' ';
        }
        
        *dst = '\0';
        
        // Отправляем результат родителю
        write(STDOUT_FILENO, result, strlen(result));
    }
    return EXIT_SUCCESS;
}