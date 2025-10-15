#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int main(int argc, char *argv[]) {
    if (argc != 1) {
        printf("Usage: %s\n", argv[0]);
        printf("No arguments needed. Program reads from stdin and writes to stdout.\n");
        return EXIT_FAILURE;
    }

    int pipe_parent_to_child1[2];
    int pipe_child1_to_child2[2];
    int pipe_child2_to_parent[2];

    // Создаем каналы
    if (pipe(pipe_parent_to_child1) == -1 || 
        pipe(pipe_child1_to_child2) == -1 || 
        pipe(pipe_child2_to_parent) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Создаем Child1
    pid_t child1 = fork();
    if (child1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child1 == 0) {
        close(pipe_parent_to_child1[1]);
        close(pipe_child1_to_child2[0]);
        close(pipe_child2_to_parent[0]);
        close(pipe_child2_to_parent[1]);

        dup2(pipe_parent_to_child1[0], STDIN_FILENO);
        dup2(pipe_child1_to_child2[1], STDOUT_FILENO);

        close(pipe_parent_to_child1[0]);
        close(pipe_child1_to_child2[1]);

        execl("./child1", "child1", NULL);
        perror("execl child1");
        exit(EXIT_FAILURE);
    }

    // Создаем Child2
    pid_t child2 = fork();
    if (child2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child2 == 0) {
        close(pipe_parent_to_child1[0]);
        close(pipe_parent_to_child1[1]);
        close(pipe_child1_to_child2[1]);
        close(pipe_child2_to_parent[0]);

        dup2(pipe_child1_to_child2[0], STDIN_FILENO);
        dup2(pipe_child2_to_parent[1], STDOUT_FILENO);

        close(pipe_child1_to_child2[0]);
        close(pipe_child2_to_parent[1]);

        execl("./child2", "child2", NULL);
        perror("execl child2");
        exit(EXIT_FAILURE);
    }

    close(pipe_parent_to_child1[0]);
    close(pipe_child1_to_child2[0]);
    close(pipe_child1_to_child2[1]);
    close(pipe_child2_to_parent[1]);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("The parent process is running. Enter the lines (Ctrl+D to end):\n");

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        write(pipe_parent_to_child1[1], buffer, strlen(buffer));

        bytes_read = read(pipe_child2_to_parent[0], buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Result: %s\n", buffer);
        }
    }

    close(pipe_parent_to_child1[1]);
    close(pipe_child2_to_parent[0]);

    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    printf("The parent process is completed.\n");
    return EXIT_SUCCESS;
}