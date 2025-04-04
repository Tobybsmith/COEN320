#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include "Shared_mem.h"

#define SHM_NAME "/Shared_mem"

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    SharedMemory* sharedMem = (SharedMemory*) mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedMem == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&sharedMem->simClock.clockMutex, &mutexAttr);
    pthread_mutex_init(&sharedMem->radarDataMutex, &mutexAttr);
    pthread_mutex_init(&sharedMem->displayConsoleMutex, &mutexAttr);

    sharedMem->simClock.currentTimeInSeconds = 0;
    sharedMem->psrDataCount = 0;
    sharedMem->ssrDataCount = 0;
    sharedMem->displayDetailId = -1;

    std::cout << "Shared memory initialized. Press Ctrl+C to terminate.\n";

    while (true) {
        sleep(1);
    }

    munmap(sharedMem, sizeof(SharedMemory));
    shm_unlink(SHM_NAME);
    return 0;
}
