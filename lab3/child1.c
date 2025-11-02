#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#define SHM_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 5) {
        return EXIT_FAILURE;
    }
    
    const char *SHM_NAME = argv[1];
    const char *SEM_PARENT = argv[2];
    const char *SEM_CHILD1 = argv[3];
    const char *SEM_CHILD2 = argv[4];
    
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shm_fd == -1) {
        return EXIT_FAILURE;
    }
    
    char *shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        return EXIT_FAILURE;
    }
    
    sem_t *sem_parent = sem_open(SEM_PARENT, O_RDWR);
    if (sem_parent == SEM_FAILED) {
        return EXIT_FAILURE;
    }
    
    sem_t *sem_child1 = sem_open(SEM_CHILD1, O_RDWR);
    if (sem_child1 == SEM_FAILED) {
        return EXIT_FAILURE;
    }
    
    sem_t *sem_child2 = sem_open(SEM_CHILD2, O_RDWR);
    if (sem_child2 == SEM_FAILED) {
        return EXIT_FAILURE;
    }
    
    bool running = true;
    
    while (running) {
        sem_wait(sem_child1);
        
        uint32_t *data_size = (uint32_t *)shm_ptr;
        char *data = shm_ptr + sizeof(uint32_t);
        
        if (*data_size == UINT32_MAX) {
            running = false;
            sem_post(sem_child2);
        } else if (*data_size > 0) {
            for (uint32_t i = 0; i < *data_size; i++) {
                data[i] = toupper(data[i]);
            }
            sem_post(sem_child2);
        }
    }
    
    sem_close(sem_parent);
    sem_close(sem_child1);
    sem_close(sem_child2);
    
    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    
    return EXIT_SUCCESS;
}