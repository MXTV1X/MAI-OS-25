#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <wait.h>
#include <semaphore.h>
#include <stdio.h>

#define SHM_SIZE 4096
#define BUFFER_SIZE 1024

char SHM_NAME[1024];
char SEM_PARENT[1024];
char SEM_CHILD1[1024];
char SEM_CHILD2[1024];

int main() {
    char unique_suffix[64];
    snprintf(unique_suffix, sizeof(unique_suffix), "%d", getpid());
    
    snprintf(SHM_NAME, sizeof(SHM_NAME), "shm-%s", unique_suffix);
    snprintf(SEM_PARENT, sizeof(SEM_PARENT), "sem-parent-%s", unique_suffix);
    snprintf(SEM_CHILD1, sizeof(SEM_CHILD1), "sem-child1-%s", unique_suffix);
    snprintf(SEM_CHILD2, sizeof(SEM_CHILD2), "sem-child2-%s", unique_suffix);
    
    int shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    
    char *shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    
    uint32_t *data_size = (uint32_t *)shm_ptr;
    *data_size = 0;
    
    sem_t *sem_parent = sem_open(SEM_PARENT, O_CREAT | O_EXCL, 0600, 1);
    if (sem_parent == SEM_FAILED) {
        perror("sem_open (parent)");
        exit(EXIT_FAILURE);
    }
    
    sem_t *sem_child1 = sem_open(SEM_CHILD1, O_CREAT | O_EXCL, 0600, 0);
    if (sem_child1 == SEM_FAILED) {
        perror("sem_open (child1)");
        exit(EXIT_FAILURE);
    }
    
    sem_t *sem_child2 = sem_open(SEM_CHILD2, O_CREAT | O_EXCL, 0600, 0);
    if (sem_child2 == SEM_FAILED) {
        perror("sem_open (child2)");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid1 = fork();
    if (pid1 == 0) {
        char *args[] = {"./child1", SHM_NAME, SEM_PARENT, SEM_CHILD1, SEM_CHILD2, NULL};
        execv("./child1", args);
        perror("execv child1");
        exit(EXIT_FAILURE);
    } else if (pid1 < 0) {
        perror("fork child1");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        char *args[] = {"./child2", SHM_NAME, SEM_PARENT, SEM_CHILD1, SEM_CHILD2, NULL};
        execv("./child2", args);
        perror("execv child2");
        exit(EXIT_FAILURE);
    } else if (pid2 < 0) {
        perror("fork child2");
        exit(EXIT_FAILURE);
    }
    
    bool running = true;
    char buffer[BUFFER_SIZE];
    
    printf("Enter text (Ctrl+D for exit): ");
    fflush(stdout);
    
    while (running) {
        sem_wait(sem_parent);
        
        data_size = (uint32_t *)shm_ptr;
        char *data = shm_ptr + sizeof(uint32_t);
        
        if (*data_size == UINT32_MAX) {
            running = false;
            sem_post(sem_child1);
        } else if (*data_size > 0) {
            printf("Result: %.*s\n\n", *data_size, data);
            *data_size = 0;
            
            printf("Enter text (Ctrl+D for exit): ");
            fflush(stdout);
            
            if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len-1] == '\n') {
                    buffer[len-1] = '\0';
                    len--;
                }
                *data_size = len;
                memcpy(data, buffer, len);
                sem_post(sem_child1);
            } else {
                *data_size = UINT32_MAX;
                running = false;
                sem_post(sem_child1);
            }
        } else {
            if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
                size_t len = strlen(buffer);
                if (len > 0 && buffer[len-1] == '\n') {
                    buffer[len-1] = '\0';
                    len--;
                }
                *data_size = len;
                memcpy(data, buffer, len);
                sem_post(sem_child1);
            } else {
                *data_size = UINT32_MAX;
                running = false;
                sem_post(sem_child1);
            }
        }
    }

    sleep(1);
    
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    
    sem_close(sem_parent);
    sem_close(sem_child1);
    sem_close(sem_child2);
    
    sem_unlink(SEM_PARENT);
    sem_unlink(SEM_CHILD1);
    sem_unlink(SEM_CHILD2);
    
    munmap(shm_ptr, SHM_SIZE);
    shm_unlink(SHM_NAME);
    close(shm_fd);
    
    return EXIT_SUCCESS;
}