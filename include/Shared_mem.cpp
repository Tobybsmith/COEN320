#include <iostream>
#include <fcntl.h>       // For O_* constants
#include <sys/mman.h>    // For shm_open, mmap
#include <unistd.h>      // For ftruncate
#include <cstring>       // For memset
#include <cstdlib>       // For exit
#include "Shared_mem.h"

#define SHM_NAME "/Shared_mem"

int main() {
    // create or open the shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // set the size of the shared memory object
    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    // map the shared memory object into the process’s address space
    SharedMemory* sharedMem = (SharedMemory*) mmap(
        NULL,
        sizeof(SharedMemory),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm_fd,
        0
    );

    if (sharedMem == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // initialize mutex attributes for inter-process sharing
    pthread_mutexattr_t mutexAttr;
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);

    // initialize all mutexes
    pthread_mutex_init(&sharedMem->simClock.clockMutex, &mutexAttr);
    pthread_mutex_init(&sharedMem->radarDataMutex, &mutexAttr);
    //add the other mutexes here

    // initialize values
    sharedMem->simClock.currentTimeInSeconds = 0;
    sharedMem->psrDataCount = 0;
    sharedMem->ssrDataCount = 0;

    std::cout << "Shared memory segment created and initialized.\n";

    // Optional: Keep the process alive to keep shared memory mapped
    // For now, just pause (e.g., wait for enter)
    std::cout << "Press ENTER to exit and unmap shared memory.\n";
    std::cin.get();

    // Step 7: Cleanup (optional in setup phase, useful for testing)
    munmap(sharedMem, sizeof(SharedMemory));
    shm_unlink(SHM_NAME);  // Only one process should unlink when it's no longer needed

    return 0;
}
